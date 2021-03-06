#include "u.h"
#include "lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"

#include "ip.h"

static ushort tftpport = 5000;
static int Id = 1;
static Netaddr myaddr;
static Netaddr server;

typedef struct {
	uchar	header[4];
	uchar	data[Segsize];
} Tftp;
static Tftp tftpb;

static void
hnputs(uchar *ptr, ushort val)
{
	ptr[0] = val>>8;
	ptr[1] = val;
}

static void
hnputl(uchar *ptr, ulong val)
{
	ptr[0] = val>>24;
	ptr[1] = val>>16;
	ptr[2] = val>>8;
	ptr[3] = val;
}

static ulong
nhgetl(uchar *ptr)
{
	return ((ptr[0]<<24) | (ptr[1]<<16) | (ptr[2]<<8) | ptr[3]);
}

static ushort
nhgets(uchar *ptr)
{
	return ((ptr[0]<<8) | ptr[1]);
}

static	short	endian	= 1;
static	char*	aendian	= (char*)&endian;
#define	LITTLE	*aendian

static ushort
ptcl_csum(void *a, int len)
{
	uchar *addr;
	ulong t1, t2;
	ulong losum, hisum, mdsum, x;

	addr = a;
	losum = 0;
	hisum = 0;
	mdsum = 0;

	x = 0;
	if((ulong)addr & 1) {
		if(len) {
			hisum += addr[0];
			len--;
			addr++;
		}
		x = 1;
	}
	while(len >= 16) {
		t1 = *(ushort*)(addr+0);
		t2 = *(ushort*)(addr+2);	mdsum += t1;
		t1 = *(ushort*)(addr+4);	mdsum += t2;
		t2 = *(ushort*)(addr+6);	mdsum += t1;
		t1 = *(ushort*)(addr+8);	mdsum += t2;
		t2 = *(ushort*)(addr+10);	mdsum += t1;
		t1 = *(ushort*)(addr+12);	mdsum += t2;
		t2 = *(ushort*)(addr+14);	mdsum += t1;
		mdsum += t2;
		len -= 16;
		addr += 16;
	}
	while(len >= 2) {
		mdsum += *(ushort*)addr;
		len -= 2;
		addr += 2;
	}
	if(x) {
		if(len)
			losum += addr[0];
		if(LITTLE)
			losum += mdsum;
		else
			hisum += mdsum;
	} else {
		if(len)
			hisum += addr[0];
		if(LITTLE)
			hisum += mdsum;
		else
			losum += mdsum;
	}

	losum += hisum >> 8;
	losum += (hisum & 0xff) << 8;
	while(hisum = losum>>16)
		losum = hisum + (losum & 0xffff);

	return ~losum;
}

static ushort
ip_csum(uchar *addr)
{
	int len;
	ulong sum = 0;

	len = (addr[0]&0xf)<<2;

	while(len > 0) {
		sum += addr[0]<<8 | addr[1] ;
		len -= 2;
		addr += 2;
	}

	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);
	return (sum^0xffff);
}

static void
udpsend(int ctlrno, Netaddr *a, void *data, int dlen)
{
	Udphdr *uh;
	Etherhdr *ip;
	Etherpkt pkt;
	int len, ptcllen;


	uh = (Udphdr*)&pkt;

	memset(uh, 0, sizeof(Etherpkt));
	memmove(uh->udpcksum+sizeof(uh->udpcksum), data, dlen);

	/*
	 * UDP portion
	 */
	ptcllen = dlen + (UDP_HDRSIZE-UDP_PHDRSIZE);
	uh->ttl = 0;
	uh->udpproto = IP_UDPPROTO;
	uh->frag[0] = 0;
	uh->frag[1] = 0;
	hnputs(uh->udpplen, ptcllen);
	hnputl(uh->udpsrc, myaddr.ip);
	hnputs(uh->udpsport, myaddr.port);
	hnputl(uh->udpdst, a->ip);
	hnputs(uh->udpdport, a->port);
	hnputs(uh->udplen, ptcllen);
	uh->udpcksum[0] = 0;
	uh->udpcksum[1] = 0;
	dlen = (dlen+1)&~1;
	hnputs(uh->udpcksum, ptcl_csum(&uh->ttl, dlen+UDP_HDRSIZE));

	/*
	 * IP portion
	 */
	ip = (Etherhdr*)&pkt;
	len = sizeof(Udphdr)+dlen;
	ip->vihl = IP_VER|IP_HLEN;
	ip->tos = 0;
	ip->ttl = 255;
	hnputs(ip->length, len-ETHER_HDR);
	hnputs(ip->id, Id++);
	ip->frag[0] = 0;
	ip->frag[1] = 0;
	ip->cksum[0] = 0;
	ip->cksum[1] = 0;
	hnputs(ip->cksum, ip_csum(&ip->vihl));

	/*
	 * Ethernet MAC portion
	 */
	hnputs(ip->type, ET_IP);
	memmove(ip->d, a->ea, sizeof(ip->d));

	ethertxpkt(ctlrno, &pkt, len, Timeout);
}

static void
nak(int ctlrno, Netaddr *a, int code, char *msg, int report)
{
	int n;
	char buf[128];

	buf[0] = 0;
	buf[1] = Tftp_ERROR;
	buf[2] = 0;
	buf[3] = code;
	strcpy(buf+4, msg);
	n = strlen(msg) + 4 + 1;
	udpsend(ctlrno, a, buf, n);
	if(report)
		print("\ntftp: error(%d): %s\n", code, msg);
}

static void
dumpbytes(uchar *p, int n)
{
	while(n-- > 0)
		print("%2.2ux ", *p++);
	print("\n");
}

static int
udprecv(int ctlrno, Netaddr *a, void *data, int dlen)
{
	int n, len;
	ushort csm;
	Udphdr *h;
	ulong addr;
	Etherpkt pkt;

	for(;;) {
		n = etherrxpkt(ctlrno, &pkt, Timeout);
		if(n <= 0)
			return 0;

		h = (Udphdr*)&pkt;
		if(nhgets(h->type) != ET_IP)
			continue;

		if(ip_csum(&h->vihl)) {
			print("ip chksum error\n");
			continue;
		}
		if(h->vihl != (IP_VER|IP_HLEN)) {
			print("ip bad vers/hlen\n");
			continue;
		}

		if(h->udpproto != IP_UDPPROTO)
			continue;

		h->ttl = 0;
		len = nhgets(h->udplen);
		hnputs(h->udpplen, len);

		if(nhgets(h->udpcksum)) {
			csm = ptcl_csum(&h->ttl, len+UDP_PHDRSIZE);
			if(csm != 0) {
				print("udp chksum error csum #%4lux len %d\n", csm, n);
				dumpbytes((uchar*)&pkt, n < 64 ? n : 64);
				break;
			}
		}

		if(a->port != 0 && nhgets(h->udpsport) != a->port)
			continue;

		addr = nhgetl(h->udpsrc);
		if(a->ip != Bcastip && addr != a->ip)
			continue;

		len -= UDP_HDRSIZE-UDP_PHDRSIZE;
		if(len > dlen) {
			print("udp: packet too big\n");
			continue;
		}

		memmove(data, h->udpcksum+sizeof(h->udpcksum), len);
		a->ip = addr;
		a->port = nhgets(h->udpsport);
		memmove(a->ea, pkt.s, sizeof(a->ea));

		return len;
	}

	return 0;
}

static int tftpblockno;

static int
tftpopen(int ctlrno, Netaddr *a, char *name, Tftp *tftp)
{
	int i, len, rlen;
	char buf[Segsize+2];

	buf[0] = 0;
	buf[1] = Tftp_READ;
	len = sprint(buf+2, "%s", name) + 2;
	len += sprint(buf+len+1, "octet") + 2;

	for(i = 0; i < 5; i++){
		udpsend(ctlrno, a, buf, len);
		a->port = 0;
		if((rlen = udprecv(ctlrno, a, tftp, sizeof(Tftp))) < sizeof(tftp->header))
			continue;

		switch((tftp->header[0]<<8)|tftp->header[1]){

		case Tftp_ERROR:
			print("tftpopen: error (%d): %s\n",
				(tftp->header[2]<<8)|tftp->header[3], tftp->data);
			return -1;

		case Tftp_DATA:
			tftpblockno = 1;
			len = (tftp->header[2]<<8)|tftp->header[3];
			if(len != tftpblockno){
				print("tftpopen: block error: %d\n", len);
				nak(ctlrno, a, 1, "block error", 0);
				return -1;
			}
			return rlen-sizeof(tftp->header);
		}
	}

	print("tftpopen: failed to connect to server\n");
	return -1;
}

static int
tftpread(int ctlrno, Netaddr *a, Tftp *tftp, int dlen)
{
	int blockno, len;
	uchar buf[4];

	buf[0] = 0;
	buf[1] = Tftp_ACK;
	buf[2] = tftpblockno>>8;
	buf[3] = tftpblockno;
	tftpblockno++;

	dlen += sizeof(tftp->header);

buggery:
	udpsend(ctlrno, a, buf, sizeof(buf));

	if((len = udprecv(ctlrno, a, tftp, dlen)) != dlen){
		print("tftpread: %d != %d\n", len, dlen);
		nak(ctlrno, a, 2, "short read", 0);
	}

	blockno = (tftp->header[2]<<8)|tftp->header[3];
	if(blockno != tftpblockno){
		print("tftpread: block error: %d, expected %d\n", blockno, tftpblockno);

		if(blockno == tftpblockno-1)
			goto buggery;
		nak(ctlrno, a, 1, "block error", 0);

		return -1;
	}

	return len-sizeof(tftp->header);
}

static void
tftpclose(int ctlrno, Netaddr *a, uchar code, char *msg)
{
	nak(ctlrno, a, code, msg, 0);
}

int
bootp(int ctlrno, char *file)
{
	Bootp req, rep;
	int i, dlen, segsize, text, data, bss, total;
	uchar *ea, *addr, *p;
	ulong entry;
	Exec *exec;
	char name[128], *filename, *sysname;

	if((ea = etheraddr(ctlrno)) == 0){
		print("invalid ctlrno %d\n", ctlrno);
		return -1;
	}

	filename = 0;
	sysname = 0;
	if(file && *file){
		strcpy(name, file);
		if(filename = strchr(name, ':')){
			if(filename != name && *(filename-1) != '\\'){
				sysname = name;
				*filename++ = 0;
			}
		}
		else
			filename = name;
	}
		

	memset(&req, 0, sizeof(req));
	req.op = Bootrequest;
	req.htype = 1;			/* ethernet */
	req.hlen = Eaddrlen;		/* ethernet */
	memmove(req.chaddr, ea, Eaddrlen);

	myaddr.ip = 0;
	myaddr.port = BPportsrc;
	memmove(myaddr.ea, ea, sizeof(myaddr.ea));

	for(i = 0; i < 10; i++) {
		server.ip = Bcastip;
		server.port = BPportdst;
		memset(server.ea, 0xFF, sizeof(server.ea));
		udpsend(ctlrno, &server, &req, sizeof(req));
		if(udprecv(ctlrno, &server, &rep, sizeof(rep)) <= 0)
			continue;
		if(memcmp(req.chaddr, rep.chaddr, Eaddrlen))
			continue;
		if(rep.htype != 1 || rep.hlen != Eaddrlen)
			continue;
		if(sysname == 0 || strcmp(sysname, rep.sname) == 0)
			break;
	}
	if(i >= 10) {
		print("bootp timed out\n");
		return -1;
	}

	if(filename == 0 || *filename == 0)
		filename = rep.file;
	if(rep.sname[0] != '\0')
		print("%s:%s\n", rep.sname, filename);

	myaddr.ip = nhgetl(rep.yiaddr);
	myaddr.port = tftpport++;
	server.port = TFTPport;

	if((dlen = tftpopen(ctlrno, &server, filename, &tftpb)) < 0)
		return -1;

	exec = (Exec*)(tftpb.data);
	if(dlen < sizeof(Exec) || GLLONG(exec->magic) != I_MAGIC){
		nak(ctlrno, &server, 0, "bad magic number", 1);
		return -1;
	}
	text = GLLONG(exec->text);
	data = GLLONG(exec->data);
	bss = GLLONG(exec->bss);
	total = text+data+bss;
	entry = GLLONG(exec->entry);
	print("%d", text);

	addr = (uchar*)PADDR(entry);
	p = tftpb.data+sizeof(Exec);
	dlen -= sizeof(Exec);
	segsize = text;
	for(;;){
		if(dlen == 0){
			if((dlen = tftpread(ctlrno, &server, &tftpb, sizeof(tftpb.data))) < 0)
				return -1;
			p = tftpb.data;
		}
		if(segsize <= dlen)
			i = segsize;
		else
			i = dlen;
		memmove(addr, p, i);

		addr += i;
		p += i;
		segsize -= i;
		dlen -= i;

		if(segsize <= 0){
			if(data == 0)
				break;
			print("+%d", data);
			segsize = data;
			data = 0;
			addr = (uchar*)PGROUND((ulong)addr);
		}
	}
	tftpclose(ctlrno, &server, 3, "ok");
	print("+%d=%d\n", bss, total);
	print("entry: 0x%lux\n", entry);

	(*(void(*)(void))(PADDR(entry)))();
	
	return 0;
}

#include	"l.h"

void
dodata(void)
{
	int i;
	Sym *s;
	Prog *p;
	long t, u;

	if(debug['v'])
		Bprint(&bso, "%5.2f dodata\n", cputime());
	Bflush(&bso);
	for(p = datap; p != P; p = p->link) {
		s = p->from.sym;
		if(s->type == SBSS)
			s->type = SDATA;
		if(s->type != SDATA)
			diag("initialize non-data (%d): %s\n%P\n",
				s->type, s->name, p);
		t = p->from.offset + p->width;
		if(t > s->value)
			diag("initialize bounds (%ld): %s\n%P\n",
				s->value, s->name, p);
	}

	/* allocate small guys */
	datsize = 0;
	for(i=0; i<NHASH; i++)
	for(s = hash[i]; s != S; s = s->link) {
		if(s->type != SDATA)
		if(s->type != SBSS)
			continue;
		t = s->value;
		if(t == 0) {
			diag("%s: no size\n", s->name);
			t = 1;
		}
		t = rnd(t, 4);;
		s->value = t;
		if(t > MINSIZ)
			continue;
		s->value = datsize;
		datsize += t;
		s->type = SDATA1;
	}

	/* allocate the rest of the data */
	for(i=0; i<NHASH; i++)
	for(s = hash[i]; s != S; s = s->link) {
		if(s->type != SDATA) {
			if(s->type == SDATA1)
				s->type = SDATA;
			continue;
		}
		t = s->value;
		s->value = datsize;
		datsize += t;
	}

	if(debug['j']) {
		/*
		 * pad data with bss that fits up to next
		 * 8k boundary, then push data to 8k
		 */
		u = rnd(datsize, 8192);
		u -= datsize;
		for(i=0; i<NHASH; i++)
		for(s = hash[i]; s != S; s = s->link) {
			if(s->type != SBSS)
				continue;
			t = s->value;
			if(t > u)
				continue;
			u -= t;
			s->value = datsize;
			s->type = SDATA;
			datsize += t;
		}
		datsize += u;
	}

	/* now the bss */
	bsssize = 0;
	for(i=0; i<NHASH; i++)
	for(s = hash[i]; s != S; s = s->link) {
		if(s->type != SBSS)
			continue;
		t = s->value;
		s->value = bsssize + datsize;
		bsssize += t;
	}

	symSB = lookup("setSB", 0);
	xdefine("setSB", SDATA, 0L);
	xdefine("bdata", SDATA, 0L);
	xdefine("edata", SDATA, datsize);
	xdefine("end", SBSS, datsize+bsssize);
	xdefine("etext", STEXT, 0L);
}

Prog*
brchain(Prog *p)
{
	int i;

	for(i=0; i<20; i++) {
		if(p == P || p->as != AB)
			return p;
		p = p->cond;
	}
	return P;
}

void
follow(void)
{

	if(debug['v'])
		Bprint(&bso, "%5.2f follow\n", cputime());
	Bflush(&bso);
	firstp = prg();
	lastp = firstp;
	xfol(textp);
	firstp = firstp->link;
	lastp->link = P;
}

void
xfol(Prog *p)
{
	Prog *q;
	int i;
	enum as a;

loop:
	if(p == P)
		return;
	if(p->as == ATEXT)
		curtext = p;
	if(p->as == AB)
	if((q = p->cond) != P) {
		p->mark = 1;
		p = q;
		if(p->mark == 0)
			goto loop;
	}
	if(p->mark) {
		/* copy up to 4 instructions to avoid branch */
		for(i=0,q=p; i<4; i++,q=q->link) {
			if(q == P)
				break;
			if(q == lastp)
				break;
			a = q->as;
			if(a == ANOP) {
				i--;
				continue;
			}
			if(a == AB || a == ARTS)
				break;
			if(q->cond == P || q->cond->mark)
				continue;
			if(a == ABAL)
				continue;
			for(;;) {
				if(p->as == ANOP) {
					p = p->link;
					continue;
				}
				q = copyp(p);
				p = p->link;
				q->mark = 1;
				lastp->link = q;
				lastp = q;
				if(q->as != a || q->cond == P || q->cond->mark)
					continue;
				q->as = relinv(q->as);
				p = q->cond;
				q->cond = q->link;
				q->link = p;
				xfol(q->link);
				p = q->link;
				if(p->mark)
					return;
				goto loop;
			}
		} /* */
		q = prg();
		q->as = AB;
		q->line = p->line;
		q->to.type = D_BRANCH;
		q->to.offset = p->pc;
		q->cond = p;
		p = q;
	}
	p->mark = 1;
	lastp->link = p;
	lastp = p;
	a = p->as;
	if(a == AB || a == ARTS)
		return;
	if(p->cond != P)
	if(a != ABAL) {
		q = brchain(p->link);
		if(q != P && q->mark) {
			p->as = relinv(a);
			p->link = p->cond;
			p->cond = q;
		}
		xfol(p->link);
		q = brchain(p->cond);
		if(q->mark) {
			p->cond = q;
			return;
		}
		p = q;
		goto loop;
	}
	p = p->link;
	goto loop;
}

int
relinv(int a)
{
	switch(a) {
	case ABE:	return ABNE;
	case ABNE:	return ABE;
	case ABLE:	return ABG;
	case ABL:	return ABGE;
	case ABGE:	return ABL;
	case ABG:	return ABLE;
	case ABBS:	return ABBC;
	case ABBC:	return ABBS;
	case ABO:	return ABNO;
	case ABNO:	return ABO;
	}
	diag("unknown relation: %s in %s\n", anames[a], TNAME);
	return a;
}

void
doinit(void)
{
	Sym *s;
	Prog *p;
	int x;

	for(p = datap; p != P; p = p->link) {
		x = p->to.type;
		if(x != D_EXTERN && x != D_STATIC)
			continue;
		s = p->to.sym;
		if(s->type == 0 || s->type == SXREF)
			diag("undefined %s initializer of %s\n",
				s->name, p->from.sym->name);
		p->to.offset += s->value;
		p->to.type = D_CONST;
		if(s->type == SDATA || s->type == SBSS)
			p->to.offset += INITDAT;
	}
}

void
patch(void)
{
	long c;
	Prog *p, *q;
	Sym *s;
	long vexit;

	if(debug['v'])
		Bprint(&bso, "%5.2f mkfwd\n", cputime());
	Bflush(&bso);
	mkfwd();
	if(debug['v'])
		Bprint(&bso, "%5.2f patch\n", cputime());
	Bflush(&bso);
	s = lookup("exit", 0);
	vexit = s->value;
	for(p = firstp; p != P; p = p->link) {
		if(p->as == ATEXT)
			curtext = p;
		if(p->as == ABAL || p->as == ARTS) {
			s = p->to.sym;
			if(s) {
				if(s->type != STEXT && s->type != SLEAF) {
					diag("undefined: %s in %s\n", s->name, TNAME);
					s->type = STEXT;
					s->value = vexit;
				}
				p->to.offset = s->value;
				p->to.type = D_BRANCH;
			}
		}
		if(p->to.type != D_BRANCH)
			continue;
		c = p->to.offset;
		for(q = firstp; q != P;) {
			if(q->forwd != P)
			if(c >= q->forwd->pc) {
				q = q->forwd;
				continue;
			}
			if(c == q->pc)
				break;
			q = q->link;
		}
		if(q == P) {
			diag("branch out of range in %s\n%P\n", TNAME, p);
			p->to.type = D_NONE;
		}
		p->cond = q;
	}

	for(p = firstp; p != P; p = p->link) {
		if(p->as == ATEXT)
			curtext = p;
		p->mark = 0;	/* initialization for follow */
		if(p->cond != P) {
			p->cond = brloop(p->cond);
			if(p->cond != P)
			if(p->to.type == D_BRANCH)
				p->to.offset = p->cond->pc;
		}
	}
}

#define	LOG	5
void
mkfwd(void)
{
	Prog *p;
	int i;
	long dwn[LOG], cnt[LOG];
	Prog *lst[LOG];

	for(i=0; i<LOG; i++) {
		if(i == 0)
			cnt[i] = 1; else
			cnt[i] = LOG * cnt[i-1];
		dwn[i] = 1;
		lst[i] = P;
	}
	i = 0;
	for(p = firstp; p != P; p = p->link) {
		if(p->as == ATEXT)
			curtext = p;
		i--;
		if(i < 0)
			i = LOG-1;
		p->forwd = P;
		dwn[i]--;
		if(dwn[i] <= 0) {
			dwn[i] = cnt[i];
			if(lst[i] != P)
				lst[i]->forwd = p;
			lst[i] = p;
		}
	}
}

Prog*
brloop(Prog *p)
{
	int c;
	Prog *q;

	c = 0;
	for(q = p; q != P; q = q->cond) {
		if(q->as != AB)
			break;
		c++;
		if(c >= 100)
			return P;
	}
	return q;
}

long
atolwhex(char *s)
{
	long n;
	int f;

	n = 0;
	f = 0;
	while(*s == ' ' || *s == '\t')
		s++;
	if(*s == '-' || *s == '+') {
		if(*s++ == '-')
			f = 1;
		while(*s == ' ' || *s == '\t')
			s++;
	}
	if(s[0]=='0' && s[1]){
		if(s[1]=='x' || s[1]=='X'){
			s += 2;
			for(;;){
				if(*s >= '0' && *s <= '9')
					n = n*16 + *s++ - '0';
				else if(*s >= 'a' && *s <= 'f')
					n = n*16 + *s++ - 'a' + 10;
				else if(*s >= 'A' && *s <= 'F')
					n = n*16 + *s++ - 'A' + 10;
				else
					break;
			}
		} else
			while(*s >= '0' && *s <= '7')
				n = n*8 + *s++ - '0';
	} else
		while(*s >= '0' && *s <= '9')
			n = n*10 + *s++ - '0';
	if(f)
		n = -n;
	return n;
}

void
undef(void)
{
	int i;
	Sym *s;

	for(i=0; i<NHASH; i++)
	for(s = hash[i]; s != S; s = s->link)
		if(s->type == SXREF)
			diag("%s: not defined\n", s->name);
}

#include "lib.h"
#include "sys9.h"
#include <string.h>
#include <errno.h>

/* make this global, so programs can look at it if errno is unilluminating */

char _plan9err[ERRLEN];

static struct errmap {
	int	errno;
	char	*ename;
} map[] = {
	/* from Brazil power errstr.h */
	{EINVAL,	"never mind"},
	{EINVAL,	"inconsistent mount"},
	{EINVAL,	"not mounted"},
	{EINVAL,	"not in union"},
	{EIO,		"mount rpc error"},
	{EIO,		"mounted device shut down"},
	{EPERM,	"mounted directory forbids creation"},
	{ENOENT,	"file does not exist"},
	{ENXIO,	"unknown device in # filename"},
	{ENOTDIR,	"not a directory"},
	{EISDIR,	"file is a directory"},
	{EINVAL,	"bad character in file name"},
	{EINVAL,	"file name syntax"},
	{EPERM,	"permission denied"},
	{EPERM,	"inappropriate use of fd"},
	{EINVAL,	"bad arg in system call"},
	{EBUSY,	"device or object already in use"},
	{EIO,		"i/o error"},
	{EIO,		"read or write too large"},
	{EIO,		"read or write too small"},
	{EINVAL,	"bad network address"},
	{EMSGSIZE,	"message is too big for protocol"},
	{EISCON,	"network device is busy or allocated"},
	{EPFNOSUPPORT,	"network protocol not supported"},
	{EADDRINUSE,	"network port not available"},
	{ENETDOWN,	"bad interface or no free interface slots"},
	{ENOTCONN,	"not announced"},
	{ESHUTDOWN,	"write to hungup stream"},
	{EINVAL,	"bad process or stream control request"},
	{EBUSY,	"no free devices"},
	{EBUSY,	"no free environment resources"},
	{ESHUTDOWN,	"mux server shut down"},
	{EBUSY,	"all mux channels busy"},
	{EIO,		"bad mux message format or mismatch"},
	{ESRCH,		"process exited"},
	{ECHILD,	"no living children"},
	{EIO,		"i/o error in demand load"},
	{ENOMEM,	"virtual memory allocation failed"},
	{EINVAL,	"illegal line discipline"},
	{EBADF,	"fd out of range or not open"},
	{ESPIPE,	"seek on a stream"},
	{ENOEXEC,	"exec header invalid"},
	{ETIMEDOUT,	"connection timed out"},
	{ECONNREFUSED,	"connection refused"},
	{ENETUNREACH,	"network unreachable"},
	{EINTR,		"interrupted"},
	{ENETDOWN,	"service required for tcp/udp/il calls"},
	{ENOMEM,	"kernel allocate failed"},
	{ENOENT,	"subfont not cached"},
	{EINVAL,	"segments overlap"},
	{EINVAL,	"mouse type already set"},
	{EIO,		"failed to recover fd"},
	{EIO,		"i/o count too small"},
	{ENOMEM,	"out of screen memory"},
	{EGREG,	"coherency problem"},
	{EINVAL,	"bad attach specifier"},

	/* extra ones from Plan 9 power errstr.h */
	{EINVAL,	"bad bitblt or bitmap request"},
	{EINVAL,	"unallocated bitmap"},
	{ENOMEM,	"out of bitmap descriptors"},
	{ENOMEM,	"out of bitmap storage"},
	{EINVAL,	"unallocated font"},
	{EGREG,	"it's a thermal problem"},

	/* from exhausted() calls in kernel */
	{ENFILE,	"no free file descriptors"},
	{EBUSY,	"no free mount devices"},
	{EBUSY,	"no free mount rpc buffer"},
	{EBUSY,		"no free segments"},
	{ENOMEM,	"no free memory"},
	{ENOBUFS,	"no free Blocks"},
	{EBUSY,	"no free routes"},

	/* from ken */
	{EINVAL,	"attach -- bad specifier"},
	{EBADF,		"unknown fid"},
	{EINVAL,	"bad character in directory name"},
	{EBADF,		"read/write -- on non open fid"},
	{EIO,		"read/write -- count too big"},
	{EIO,		"phase error -- directory entry not allocated"},
	{EIO,		"phase error -- qid does not match"},
	{EACCES,	"access permission denied"},
	{ENOENT,	"directory entry not found"},
	{EINVAL,	"open/create -- unknown mode"},
	{ENOTDIR,	"walk -- in a non-directory"},
	{ENOTDIR,	"create -- in a non-directory"},
	{EIO,		"phase error -- cannot happen"},
	{EEXIST,	"create -- file exists"},
	{EINVAL,	"create -- . and .. illegal names"},
	{ENOTEMPTY,	"remove -- directory not empty"},
	{EINVAL,	"attach -- privileged user"},
	{EPERM,		"wstat -- not owner"},
	{EPERM,		"wstat -- not in group"},
	{EINVAL,	"create/wstat -- bad character in file name"},
	{EBUSY,		"walk -- too many (system wide)"},
	{EROFS,		"file system read only"},
	{ENOSPC,	"file system full"},
	{EINVAL,	"read/write -- offset negative"},
	{EBUSY,		"open/create -- file is locked"},
	{EBUSY,		"close/read/write -- lock is broken"},

	/* from sockets */
	{ENOTSOCK,	"not a socket"},
	{EPROTONOSUPPORT,	"protocol not supported"},
	{ECONNREFUSED,	"connection refused"},
	{EAFNOSUPPORT,	"address family not supported"},
	{ENOBUFS,	"insufficient buffer space"},
	{EOPNOTSUPP,	"operation not supported"},
	{EADDRINUSE,	"address in use"},
	{EGREG,		"unnamed error message"},
};

#define NERRMAP	(sizeof(map)/sizeof(struct errmap))

/* convert last system call error to an errno */
void
_syserrno(void)
{
	int i;

	if(_ERRSTR(_plan9err) < 0)
		errno = EINVAL;
	for(i = 0; i < NERRMAP; i++)
		if(strcmp(_plan9err, map[i].ename) == 0)
			break;
	errno = (i < NERRMAP)? map[i].errno : EINVAL;
}

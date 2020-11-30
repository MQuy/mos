#include <assert.h>
#include <errno.h>
#include <sys/utsname.h>
#include <unistd.h>

_syscall1(uname, struct utsname *);
int uname(struct utsname *info)
{
	SYSCALL_RETURN_ORIGINAL(syscall_uname(info));
}

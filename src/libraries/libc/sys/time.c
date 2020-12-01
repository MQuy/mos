#include <errno.h>
#include <sys/time.h>
#include <unistd.h>

_syscall2(gettimeofday, struct timeval *restrict, void *restrict);
int gettimeofday(struct timeval *restrict tv, void *restrict buf)
{
	SYSCALL_RETURN(syscall_gettimeofday(tv, buf));
}

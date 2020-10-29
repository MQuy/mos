#include <sys/times.h>
#include <time.h>
#include <unistd.h>

_syscall1(times, struct tms*);
clock_t times(struct tms* buf)
{
	return syscall_times(buf);
}

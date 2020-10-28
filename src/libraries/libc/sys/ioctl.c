#include <sys/ioctl.h>
#include <unistd.h>

_syscall3(ioctl, int, unsigned int, unsigned long);
int ioctl(int fd, unsigned int cmd, unsigned long arg)
{
	return syscall_ioctl(fd, cmd, arg);
}

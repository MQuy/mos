#include <stdarg.h>
#include <sys/ioctl.h>
#include <unistd.h>

_syscall3(ioctl, int, unsigned int, unsigned long);
int ioctl(int fd, unsigned int cmd, ...)
{
	va_list ap;
	va_start(ap, cmd);
	unsigned long arg = va_arg(ap, unsigned long);
	va_end(ap);
	return syscall_ioctl(fd, cmd, arg);
}

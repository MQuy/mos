#include <sys/ioctl.h>
#include <unistd.h>

int ioctl(int fd, unsigned int cmd, unsigned long arg)
{
	return _ioctl(fd, cmd, arg);
}

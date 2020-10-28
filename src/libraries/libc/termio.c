#include <sys/ioctl.h>
#include <termio.h>

int tcgetattr(int fd, struct termios *term)
{
	return ioctl(fd, TCGETS, (unsigned long)term);
}

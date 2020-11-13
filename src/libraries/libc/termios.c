#include <sys/ioctl.h>
#include <termios.h>

int tcsetattr(int fd, int opts, const struct termios *tp)
{
	return ioctl(fd, opts, (unsigned long)tp);
}

int tcflow(int fd, int action)
{
	return ioctl(fd, TCXONC, action);
}

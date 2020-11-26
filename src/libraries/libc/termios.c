#include <assert.h>
#include <sys/ioctl.h>
#include <termios.h>

int tcsetattr(int fd, int opts, const struct termios *tp)
{
	int cmd;
	switch (opts)
	{
	case TCSANOW:
		cmd = TCSETS;
		break;
	case TCSADRAIN:
		cmd = TCSETSW;
		break;
	case TCSAFLUSH:
		cmd = TCSETSF;
		break;
	default:
		assert_not_reached();
	}

	return ioctl(fd, cmd, tp);
}

int tcflow(int fd, int action)
{
	return ioctl(fd, TCXONC, action);
}

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

speed_t cfgetispeed(const struct termios *termios_p)
{
	assert_not_reached();
	__builtin_unreachable();
}

speed_t cfgetospeed(const struct termios *termios_p)
{
	assert_not_reached();
	__builtin_unreachable();
}

int cfsetispeed(struct termios *termios_p, speed_t speed)
{
	assert_not_reached();
	__builtin_unreachable();
}

int cfsetospeed(struct termios *termios_p, speed_t speed)
{
	assert_not_reached();
	__builtin_unreachable();
}

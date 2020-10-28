#include <termio.h>
#include <time.h>
#include <unistd.h>

int isatty(int fd)
{
	struct termios term;
	return tcgetattr(fd, &term) == 0;
}

int tcsetpgrp(int fd, pid_t pid)
{
	return ioctl(fd, TIOCSPGRP, (unsigned long)&pid);
}

pid_t tcgetpgrp(int fd)
{
	return ioctl(fd, TIOCGPGRP, 0);
}

int usleep(uint32_t usec)
{
	struct timespec req = {.tv_sec = usec / 1000, .tv_nsec = usec * 1000};
	return nanosleep(&req, NULL);
}

int sleep(uint32_t sec)
{
	return usleep(sec * 1000);
}

int getdents(unsigned int fd, struct dirent *dirent, unsigned int count)
{
	return syscall_getdents(fd, dirent, count);
}

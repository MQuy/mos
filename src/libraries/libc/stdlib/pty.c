#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int posix_openpt(int flags)
{
	return open("/dev/ptmx", flags);
}

char *ptsname(int fd)
{
	char *name = calloc(sizeof(_PATH_DEV) + SPECNAMELEN, 1);
	getptsname(fd, name);
	return name;
}

int grantpt(int fd)
{
	(void)fd;
	return 0;
}

int unlockpt(int fd)
{
	(void)fd;
	return 0;
}

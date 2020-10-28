#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int posix_openpt(int flags)
{
	return open("/dev/ptmx", flags, 0);
}

char *ptsname(int fd)
{
	char *name = calloc(sizeof(_PATH_DEV) + SPECNAMELEN, 1);
	getptsname(fd, name);
	return name;
}

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char defaultdir[] = "/dev/shm/";

int shm_open(const char *name, int flags, mode_t mode)
{
	char *fname;
	if (name[0] == '/')
		fname = name;
	else
	{
		fname = calloc(strlen(name) + sizeof(defaultdir), sizeof(char));
		strcpy(fname, defaultdir);
		strcat(fname, name);
	}
	return open(fname, flags, mode);
}

#include <libc/string.h>
#include <libc/unistd.h>

#include "command_line.h"

int pwd(char *cwd)
{
	write(1, cwd, strlen(cwd));
	return 0;
}

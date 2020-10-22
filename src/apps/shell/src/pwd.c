#include <string.h>
#include <unistd.h>

#include "command_line.h"

int pwd(char *cwd)
{
	write(1, cwd, strlen(cwd));
	write(1, "\n", 1);
	return 0;
}

#include <libc/string.h>
#include <libc/unistd.h>

#include "command_line.h"

int pwd(char *cwd)
{
	char text[256] = {0};
	sprintf(text, "\n%s", cwd);
	write(1, text, strlen(text));
	return 0;
}

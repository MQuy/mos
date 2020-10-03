#include <libc/stdlib.h>
#include <libc/unistd.h>

void exit(int status)
{
	_exit(status);
}

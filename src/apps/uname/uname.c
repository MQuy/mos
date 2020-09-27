#include <include/ioctls.h>
#include <include/mman.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{
	write(1, "mOS 0.0.1", 9);
	return 0;
}

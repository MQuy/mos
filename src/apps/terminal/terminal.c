#include <include/ioctls.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>
#include <libc/wait.h>
#include <stdint.h>

int main(void)
{
	int fd = open("/dev/ttyS1", O_WRONLY, 0);
	write(fd, "write to ttyS1 from userspace", 29);

	return 0;
}

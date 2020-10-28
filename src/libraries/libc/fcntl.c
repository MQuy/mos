#include <fcntl.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

_syscall3(open, const char*, int32_t, mode_t);
int open(const char* path, int flags, ...)
{
	mode_t mode = 0;
	if (flags & O_CREAT)
	{
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

	return syscall_open(path, flags, mode);
}

_syscall3(fcntl, int, int, unsigned long);
int fcntl(int fd, int cmd, ...)
{
	va_list ap;
	va_start(ap, cmd);
	unsigned long arg = va_arg(ap, unsigned long);
	va_end(ap);

	return syscall_fcntl(fd, cmd, arg);
}

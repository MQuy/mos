#include <sys/stat.h>
#include <unistd.h>

_syscall2(fstat, int32_t, struct stat *);
int fstat(int fildes, struct stat *buf)
{
	return syscall_fstat(fildes, buf);
}

_syscall2(stat, const char *, struct stat *);
int stat(const char *path, struct stat *buf)
{
	return syscall_stat(path, buf);
}

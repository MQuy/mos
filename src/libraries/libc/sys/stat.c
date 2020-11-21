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

_syscall1(umask, mode_t);
mode_t umask(mode_t cmask)
{
	return syscall_umask(cmask);
}

_syscall2(chmod, const char *, mode_t);
int chmod(const char *path, mode_t mode)
{
	return syscall_chmod(path, mode);
}

_syscall2(fchmod, int, mode_t);
int fchmod(int fildes, mode_t mode)
{
	return syscall_fchmod(fildes, mode);
}

_syscall3(mknod, const char *, mode_t, dev_t);
int mknod(const char *path, mode_t mode, dev_t dev)
{
	return syscall_mknod(path, mode, dev);
}

_syscall4(mknodat, int, const char *, mode_t, dev_t);
int mknodat(int fd, const char *path, mode_t mode, dev_t dev)
{
	return syscall_mknodat(fd, path, mode, dev);
}

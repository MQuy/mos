#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

_syscall2(fstat, int32_t, struct stat *);
int fstat(int fildes, struct stat *buf)
{
	SYSCALL_RETURN(syscall_fstat(fildes, buf));
}

_syscall2(stat, const char *, struct stat *);
int stat(const char *path, struct stat *buf)
{
	SYSCALL_RETURN(syscall_stat(path, buf));
}

_syscall1(umask, mode_t);
mode_t umask(mode_t cmask)
{
	SYSCALL_RETURN_ORIGINAL(syscall_umask(cmask));
}

_syscall2(chmod, const char *, mode_t);
int chmod(const char *path, mode_t mode)
{
	SYSCALL_RETURN(syscall_chmod(path, mode));
}

_syscall2(fchmod, int, mode_t);
int fchmod(int fildes, mode_t mode)
{
	SYSCALL_RETURN(syscall_fchmod(fildes, mode));
}

_syscall3(mknod, const char *, mode_t, dev_t);
int mknod(const char *path, mode_t mode, dev_t dev)
{
	SYSCALL_RETURN(syscall_mknod(path, mode, dev));
}

_syscall4(mknodat, int, const char *, mode_t, dev_t);
int mknodat(int fd, const char *path, mode_t mode, dev_t dev)
{
	SYSCALL_RETURN(syscall_mknodat(fd, path, mode, dev));
}

int mkfifo(const char *path, mode_t mode)
{
	assert_not_reached();
	__builtin_unreachable();
}

int mkfifoat(int fd, const char *path, mode_t mode)
{
	assert_not_reached();
	__builtin_unreachable();
}

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <termio.h>
#include <time.h>
#include <unistd.h>

int isatty(int fd)
{
	struct termios term;
	return tcgetattr(fd, &term) == 0;
}

int tcsetpgrp(int fd, pid_t pid)
{
	return ioctl(fd, TIOCSPGRP, (unsigned long)&pid);
}

pid_t tcgetpgrp(int fd)
{
	pid_t pgrp;

	if (ioctl(fd, TIOCGPGRP, &pgrp) < 0)
		return -1;

	return pgrp;
}

int usleep(useconds_t usec)
{
	struct timespec req = {.tv_sec = usec / 1000, .tv_nsec = usec * 1000};
	return nanosleep(&req, NULL);
}

int sleep(unsigned int sec)
{
	return usleep(sec * 1000);
}

_syscall3(getdents, unsigned int, struct dirent *, unsigned int);
int getdents(unsigned int fd, struct dirent *dirent, unsigned int count)
{
	SYSCALL_RETURN_ORIGINAL(syscall_getdents(fd, dirent, count));
}

int getpagesize()
{
	return PAGE_SIZE;
}

_syscall0(getpgrp);
int getpgrp()
{
	SYSCALL_RETURN_ORIGINAL(syscall_getpgrp());
}

_syscall0(getpid);
int getpid()
{
	SYSCALL_RETURN_ORIGINAL(syscall_getpid());
}

_syscall0(getuid);
int getuid()
{
	SYSCALL_RETURN_ORIGINAL(syscall_getuid());
}

_syscall1(setuid, uid_t);
int setuid(uid_t uid)
{
	SYSCALL_RETURN(syscall_setuid(uid));
}

_syscall0(getegid);
int getegid()
{
	SYSCALL_RETURN_ORIGINAL(syscall_getegid());
}

_syscall0(geteuid);
int geteuid()
{
	SYSCALL_RETURN_ORIGINAL(syscall_geteuid());
}

_syscall0(getgid);
int getgid()
{
	SYSCALL_RETURN_ORIGINAL(syscall_getgid());
}

_syscall1(setgid, gid_t);
int setgid(gid_t gid)
{
	SYSCALL_RETURN(syscall_setgid(gid));
}

_syscall1(getpgid, pid_t);
int getpgid(pid_t pid)
{
	SYSCALL_RETURN_ORIGINAL(syscall_getpgid(pid));
}

_syscall0(getppid);
int getppid()
{
	SYSCALL_RETURN_ORIGINAL(syscall_getppid());
}

_syscall2(setpgid, pid_t, pid_t);
int setpgid(pid_t pid, pid_t pgid)
{
	SYSCALL_RETURN(syscall_setpgid(pid, pgid));
}

_syscall0(getsid);
int getsid()
{
	SYSCALL_RETURN_ORIGINAL(syscall_getsid());
}

_syscall0(setsid);
int setsid()
{
	SYSCALL_RETURN_ORIGINAL(syscall_setsid());
}

_syscall1(posix_spawn, char *);
int posix_spawn(char *path)
{
	SYSCALL_RETURN(syscall_posix_spawn(path));
}

_syscall2(getptsname, int, char *);
int getptsname(int fdm, char *ptsname)
{
	SYSCALL_RETURN(syscall_getptsname(fdm, ptsname));
}

_syscall2(ftruncate, int, off_t);
int ftruncate(int fd, off_t length)
{
	SYSCALL_RETURN(syscall_ftruncate(fd, length));
}

_syscall2(truncate, const char *, off_t);
int truncate(const char *name, off_t length)
{
	SYSCALL_RETURN(syscall_truncate(name, length));
}

_syscall1(pipe, int *);
int pipe(int *fildes)
{
	SYSCALL_RETURN(syscall_pipe(fildes));
}

_syscall1(sbrk, intptr_t);
int sbrk(intptr_t increment)
{
	SYSCALL_RETURN_ORIGINAL(syscall_sbrk(increment));
}

_syscall1(brk, intptr_t);
int brk(intptr_t increment)
{
	SYSCALL_RETURN(syscall_brk(increment));
}

_syscall2(dup2, int, int);
int dup2(int oldfd, int newfd)
{
	SYSCALL_RETURN_ORIGINAL(syscall_dup2(oldfd, newfd));
}

_syscall3(execve, const char *, char *const *, char *const *);
int execve(const char *pathname, char *const argv[], char *const envp[])
{
	_clear_on_exit();
	SYSCALL_RETURN_ORIGINAL(syscall_execve(pathname, argv, envp));
}

int get_argc_from_execlx(va_list ap, const char *arg0)
{
	int argc = arg0 ? 1 : 0;
	while (arg0 && va_arg(ap, const char *))
		argc++;
	return argc;
}

char **get_argv_from_execlx(va_list ap, int argc, const char *arg0)
{
	char **argv = calloc(argc + 1, sizeof(char *));
	argv[0] = arg0;
	for (int i = 1; i < argc; ++i)
		argv[i] = va_arg(ap, const char *);
	argv[argc] = NULL;

	return argv;
}

int execl(const char *path, const char *arg0, ...)
{
	va_list ap;

	va_start(ap, arg0);
	int argc = get_argc_from_execlx(ap, arg0);
	va_end(ap);

	va_start(ap, arg0);
	char **argv = get_argv_from_execlx(ap, argc, arg0);
	va_end(ap);

	int ret = execve(path, argv, environ);
	free(argv);
	return ret;
}

int execlp(const char *file, const char *arg0, ...)
{
	va_list ap;

	va_start(ap, arg0);
	int argc = get_argc_from_execlx(ap, arg0);
	va_end(ap);

	va_start(ap, arg0);
	char **argv = get_argv_from_execlx(ap, argc, arg0);
	va_end(ap);

	int ret = execvpe(file, argv, environ);
	free(argv);
	return ret;
}

int execle(const char *path, const char *arg0, ...)
{
	va_list ap;

	va_start(ap, arg0);
	int argc = get_argc_from_execlx(ap, arg0);
	va_end(ap);

	va_start(ap, arg0);
	char **argv = get_argv_from_execlx(ap, argc, arg0);
	char **envp = va_arg(ap, char **);
	va_end(ap);

	int ret = execve(path, argv, envp);
	free(argv);
	free(envp);
	return ret;
}

int execv(const char *pathname, char *const argv[])
{
	return execve(pathname, argv, environ);
}

int execvp(const char *file, char *const argv[])
{
	return execvpe(file, argv, environ);
}

int execvpe(const char *file, char *const argv[],
			char *const envp[])
{
	assert_not_reached();
	__builtin_unreachable();
}

_syscall3(lseek, int, off_t, int);
int lseek(int fd, off_t offset, int whence)
{
	SYSCALL_RETURN_ORIGINAL(syscall_lseek(fd, offset, whence));
}

_syscall1(close, int);
int close(int fd)
{
	SYSCALL_RETURN(syscall_close(fd));
}

_syscall3(write, int, const char *, size_t);
int write(int fd, const char *buf, size_t size)
{
	SYSCALL_RETURN_ORIGINAL(syscall_write(fd, buf, size));
}

_syscall3(read, int, char *, size_t);
int read(int fd, char *buf, size_t size)
{
	SYSCALL_RETURN_ORIGINAL(syscall_read(fd, buf, size));
}

_syscall1(exit, int);
void __attribute__((noreturn)) _exit(int code)
{
	syscall_exit(code);
	__builtin_unreachable();
}

_syscall0(fork);
int fork()
{
	SYSCALL_RETURN_ORIGINAL(syscall_fork());
}

_syscall1(alarm, unsigned int);
int alarm(unsigned int seconds)
{
	return syscall_alarm(seconds);
}

_syscall2(access, const char *, int);
int access(const char *path, int amode)
{
	SYSCALL_RETURN(syscall_access(path, amode));
}

_syscall4(faccessat, int, const char *, int, int);
int faccessat(int fd, const char *path, int amode, int flag)
{
	SYSCALL_RETURN(syscall_faccessat(fd, path, amode, flag));
}

_syscall1(unlink, const char *);
int unlink(const char *path)
{
	SYSCALL_RETURN(syscall_unlink(path));
}

_syscall3(unlinkat, int, const char *, int);
int unlinkat(int fd, const char *path, int flag)
{
	SYSCALL_RETURN(syscall_unlinkat(fd, path, flag));
}

int dup(int fildes)
{
	return fcntl(fildes, F_DUPFD, 0);
}

_syscall1(chdir, const char *);
int chdir(const char *path)
{
	SYSCALL_RETURN(syscall_chdir(path));
}

_syscall1(fchdir, int);
int fchdir(int fildes)
{
	SYSCALL_RETURN(syscall_fchdir(fildes));
}

_syscall2(getcwd, char *, size_t);
char *getcwd(char *buf, size_t size)
{
	if (!size)
		size = MAXPATHLEN;
	if (!buf)
		buf = calloc(size, sizeof(char));

	SYSCALL_RETURN_POINTER(syscall_getcwd(buf, size));
}

int fsync(int fd)
{
	assert_not_reached();
	__builtin_unreachable();
}

int fdatasync(int fd)
{
	assert_not_reached();
	__builtin_unreachable();
}

int link(const char *path1, const char *path2)
{
	assert_not_reached();
	__builtin_unreachable();
}

int linkat(int fd1, const char *path1, int fd2,
		   const char *path2, int flag)
{
	assert_not_reached();
	__builtin_unreachable();
}

int chown(const char *path, uid_t owner, gid_t group)
{
	assert_not_reached();
	__builtin_unreachable();
}

int fchownat(int fd, const char *path, uid_t owner, gid_t group,
			 int flag)
{
	assert_not_reached();
	__builtin_unreachable();
}

int fchown(int fildes, uid_t owner, gid_t group)
{
	assert_not_reached();
	__builtin_unreachable();
}

long fpathconf(int fildes, int name)
{
	assert_not_reached();
	__builtin_unreachable();
}

long pathconf(const char *path, int name)
{
	assert_not_reached();
	__builtin_unreachable();
}

int mkdir(const char *path, mode_t mode)
{
	assert_not_reached();
	__builtin_unreachable();
}

int mkdirat(int fd, const char *path, mode_t mode)
{
	assert_not_reached();
	__builtin_unreachable();
}

int rmdir(const char *path)
{
	assert_not_reached();
	__builtin_unreachable();
}

long sysconf(int name)
{
	assert_not_reached();
	__builtin_unreachable();
}

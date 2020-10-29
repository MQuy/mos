#include <limits.h>
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
	return ioctl(fd, TIOCGPGRP, 0);
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
	return syscall_getdents(fd, dirent, count);
}

int getpagesize()
{
	return PAGE_SIZE;
}

_syscall0(getpid);
int getpid()
{
	return syscall_getpid();
}

_syscall0(getuid);
int getuid()
{
	return syscall_getuid();
}

_syscall1(setuid, uid_t);
int setuid(uid_t uid)
{
	return syscall_setuid(uid);
}

_syscall0(getegid);
int getegid()
{
	return syscall_getegid();
}

_syscall0(geteuid);
int geteuid()
{
	return syscall_geteuid();
}

_syscall0(getgid);
int getgid()
{
	return syscall_getgid();
}

_syscall1(setgid, gid_t);
int setgid(gid_t gid)
{
	return syscall_setgid(gid);
}

_syscall0(getpgid);
int getpgid()
{
	return syscall_getpgid();
}

_syscall0(getppid);
int getppid()
{
	return syscall_getppid();
}

_syscall2(setpgid, pid_t, pid_t);
int setpgid(pid_t pid, pid_t pgid)
{
	return syscall_setpgid(pid, pgid);
}

_syscall0(getsid);
int getsid()
{
	return syscall_getsid();
}

_syscall0(setsid);
int setsid()
{
	return syscall_setsid();
}

_syscall1(posix_spawn, char *);
int posix_spawn(char *path)
{
	return syscall_posix_spawn(path);
}

_syscall2(getptsname, int, char *);
int getptsname(int fdm, char *ptsname)
{
	return syscall_getptsname(fdm, ptsname);
}

_syscall2(ftruncate, int, off_t);
int ftruncate(int fd, off_t length)
{
	return syscall_ftruncate(fd, length);
}

_syscall2(truncate, const char *, off_t);
int truncate(const char *name, off_t length)
{
	return syscall_truncate(name, length);
}

_syscall1(pipe, int *);
int pipe(int *fildes)
{
	return syscall_pipe(fildes);
}

_syscall1(sbrk, intptr_t);
int sbrk(intptr_t increment)
{
	return syscall_sbrk(increment);
}

_syscall1(brk, intptr_t);
int brk(intptr_t increment)
{
	return syscall_brk(increment);
}

_syscall2(dup2, int, int);
int dup2(int oldfd, int newfd)
{
	return syscall_dup2(oldfd, newfd);
}

_syscall3(execve, const char *, char *const *, char *const *);
int execve(const char *pathname, char *const argv[], char *const envp[])
{
	return syscall_execve(pathname, argv, envp);
}

_syscall3(lseek, int, off_t, int);
int lseek(int fd, off_t offset, int whence)
{
	return syscall_lseek(fd, offset, whence);
}

_syscall1(close, int);
int close(int fd)
{
	return syscall_close(fd);
}

_syscall3(write, int, const char *, size_t);
int write(int fd, const char *buf, size_t size)
{
	return syscall_write(fd, buf, size);
}

_syscall3(read, int, char *, size_t);
int read(int fd, char *buf, size_t size)
{
	return syscall_read(fd, buf, size);
}

_syscall1(exit, int);
void _exit(int code)
{
	syscall_exit(code);
}

_syscall0(fork);
int fork()
{
	return syscall_fork();
}

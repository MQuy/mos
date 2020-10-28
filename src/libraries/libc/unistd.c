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

int usleep(uint32_t usec)
{
	struct timespec req = {.tv_sec = usec / 1000, .tv_nsec = usec * 1000};
	return nanosleep(&req, NULL);
}

int sleep(uint32_t sec)
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

_syscall2(getptsname, int32_t, char *);
int getptsname(int32_t fdm, char *ptsname)
{
	return syscall_getptsname(fdm, ptsname);
}

_syscall2(dprintf, enum debug_level, const char *);
int dprintf(enum debug_level level, const char *out)
{
	return syscall_dprintf(level, out);
}

_syscall2(dprintln, enum debug_level, const char *);
int dprintln(enum debug_level level, const char *out)
{
	return syscall_dprintln(level, out);
}

_syscall2(ftruncate, int32_t, off_t);
int ftruncate(int32_t fd, off_t length)
{
	return syscall_ftruncate(fd, length);
}

_syscall2(truncate, const char *, off_t);
int truncate(const char *name, off_t length)
{
	return syscall_truncate(name, length);
}

_syscall1(pipe, int32_t *);
int pipe(int32_t *fildes)
{
	return syscall_pipe(fildes);
}

_syscall1(sbrk, int32_t);
int sbrk(int32_t increment)
{
	return syscall_sbrk(increment);
}

_syscall1(brk, uint32_t);
int brk(uint32_t increment)
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

_syscall1(close, uint32_t);
int close(uint32_t fd)
{
	return syscall_close(fd);
}

_syscall3(write, uint32_t, const char *, uint32_t);
int write(uint32_t fd, const char *buf, uint32_t size)
{
	return syscall_write(fd, buf, size);
}

_syscall3(read, uint32_t, char *, uint32_t);
int read(uint32_t fd, char *buf, uint32_t size)
{
	return syscall_read(fd, buf, size);
}

_syscall1(exit, int32_t);
void _exit(int32_t code)
{
	syscall_exit(code);
}

_syscall0(fork);
int fork()
{
	return syscall_fork();
}

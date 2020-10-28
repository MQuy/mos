#ifndef _LIBC_UNISTD_H
#define _LIBC_UNISTD_H

#include <dprint.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

// FIXME MQ 2020-05-12 copy define constants from https://github.com/torvalds/linux/blob/master/arch/x86/entry/syscalls/syscall_32.tbl
#define __NR_exit 1
#define __NR_fork 2
#define __NR_read 3
#define __NR_write 4
#define __NR_open 5
#define __NR_close 6
#define __NR_execve 11
#define __NR_time 13
#define __NR_brk 17
#define __NR_sbrk 18
#define __NR_lseek 19
#define __NR_getpid 20
#define __NR_setuid 23
#define __NR_getuid 24
#define __NR_kill 37
#define __NR_pipe 42
#define __NR_setgid 46
#define __NR_getgid 47
#define __NR_signal 48
#define __NR_geteuid 49
#define __NR_getegid 50
#define __NR_ioctl 54
#define __NR_fcntl 55
#define __NR_setpgid 57
#define __NR_dup2 63
#define __NR_getppid 64
#define __NR_setsid 66
#define __NR_sigaction 67
#define __NR_mmap 90
#define __NR_munmap 91
#define __NR_truncate 92
#define __NR_ftruncate 93
#define __NR_socket 97
#define __NR_connect 98
#define __NR_accept 99
#define __NR_getpriority 100
#define __NR_send 101
#define __NR_recv 102
#define __NR_sigreturn 103
#define __NR_bind 104
#define __NR_listen 105
#define __NR_stat 106
#define __NR_fstat 108
#define __NR_sigprocmask 126
#define __NR_getpgid 132
#define __NR_getdents 141
#define __NR_getsid 147
#define __NR_nanosleep 162
#define __NR_poll 168
#define __NR_clock_gettime 265
#define __NR_mq_open 277
#define __NR_mq_close (__NR_mq_open + 1)
#define __NR_mq_unlink (__NR_mq_open + 2)
#define __NR_mq_send (__NR_mq_open + 3)
#define __NR_mq_receive (__NR_mq_open + 4)
#define __NR_waitid 284
#define __NR_sendto 369
// TODO: MQ 2020-09-05 Use ioctl-FIODGNAME to get pts name
#define __NR_getptsname 370
// TODO: MQ 2020-09-16 Replace by writting to /dev/ttyS0
#define __NR_dprintf 512
#define __NR_dprintln 513
#define __NR_posix_spawn 514

#define _syscall0(name)                           \
	static inline int32_t syscall_##name()        \
	{                                             \
		int32_t ret;                              \
		__asm__ __volatile__("int $0x7F"          \
							 : "=a"(ret)          \
							 : "0"(__NR_##name)); \
		return ret;                               \
	}
#define _syscall1(name, type1)                               \
	static inline int32_t syscall_##name(type1 arg1)         \
	{                                                        \
		int32_t ret;                                         \
		__asm__ __volatile__("int $0x7F"                     \
							 : "=a"(ret)                     \
							 : "0"(__NR_##name), "b"(arg1)); \
		return ret;                                          \
	}

#define _syscall2(name, type1, type2)                                   \
	static inline int32_t syscall_##name(type1 arg1, type2 arg2)        \
	{                                                                   \
		int32_t ret;                                                    \
		__asm__ __volatile__("int $0x7F"                                \
							 : "=a"(ret)                                \
							 : "0"(__NR_##name), "b"(arg1), "c"(arg2)); \
		return ret;                                                     \
	}

#define _syscall3(name, type1, type2, type3)                                       \
	static inline int32_t syscall_##name(type1 arg1, type2 arg2, type3 arg3)       \
	{                                                                              \
		int32_t ret;                                                               \
		__asm__ __volatile__("int $0x7F"                                           \
							 : "=a"(ret)                                           \
							 : "0"(__NR_##name), "b"(arg1), "c"(arg2), "d"(arg3)); \
		return ret;                                                                \
	}

#define _syscall4(name, type1, type2, type3, type4)                                           \
	static inline int32_t syscall_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4)      \
	{                                                                                         \
		int32_t ret;                                                                          \
		__asm__ __volatile__("int $0x7F"                                                      \
							 : "=a"(ret)                                                      \
							 : "0"(__NR_##name), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4)); \
		return ret;                                                                           \
	}

#define _syscall5(name, type1, type2, type3, type4, type5)                                               \
	static inline int32_t syscall_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5)     \
	{                                                                                                    \
		int32_t ret;                                                                                     \
		__asm__ __volatile__("int $0x7F"                                                                 \
							 : "=a"(ret)                                                                 \
							 : "0"(__NR_##name), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)); \
		return ret;                                                                                      \
	}

struct dirent;

_syscall0(fork);
static inline int32_t fork()
{
	return syscall_fork();
}

_syscall1(exit, int32_t);
static inline void _exit(int32_t code)
{
	syscall_exit(code);
}

_syscall3(read, uint32_t, char *, uint32_t);
static inline int32_t read(uint32_t fd, char *buf, uint32_t size)
{
	return syscall_read(fd, buf, size);
}

_syscall3(write, uint32_t, const char *, uint32_t);
static inline int32_t write(uint32_t fd, const char *buf, uint32_t size)
{
	return syscall_write(fd, buf, size);
}

_syscall1(close, uint32_t);
static inline int32_t close(uint32_t fd)
{
	return syscall_close(fd);
}

_syscall3(lseek, int, off_t, int);
static inline int32_t lseek(int fd, off_t offset, int whence)
{
	return syscall_lseek(fd, offset, whence);
}

_syscall3(execve, const char *, char *const *, char *const *);
static inline int32_t execve(const char *pathname, char *const argv[], char *const envp[])
{
	return syscall_execve(pathname, argv, envp);
}

_syscall2(dup2, int, int);
static inline int32_t dup2(int oldfd, int newfd)
{
	return syscall_dup2(oldfd, newfd);
}

_syscall1(brk, uint32_t);
static inline int32_t brk(uint32_t increment)
{
	return syscall_brk(increment);
}

_syscall1(sbrk, int32_t);
static inline int32_t sbrk(int32_t increment)
{
	return syscall_sbrk(increment);
}

_syscall1(pipe, int32_t *);
static inline int32_t pipe(int32_t *fildes)
{
	return syscall_pipe(fildes);
}

_syscall2(truncate, const char *, off_t);
static inline int32_t truncate(const char *name, off_t length)
{
	return syscall_truncate(name, length);
}

_syscall2(ftruncate, int32_t, off_t);
static inline int32_t ftruncate(int32_t fd, off_t length)
{
	return syscall_ftruncate(fd, length);
}

_syscall0(getpid);
static inline int32_t getpid()
{
	return syscall_getpid();
}

_syscall0(getuid);
static inline int32_t getuid()
{
	return syscall_getuid();
}

_syscall1(setuid, uid_t);
static inline int32_t setuid(uid_t uid)
{
	return syscall_setuid(uid);
}

_syscall0(getegid);
static inline int32_t getegid()
{
	return syscall_getegid();
}

_syscall0(geteuid);
static inline int32_t geteuid()
{
	return syscall_geteuid();
}

_syscall0(getgid);
static inline int32_t getgid()
{
	return syscall_getgid();
}

_syscall1(setgid, gid_t);
static inline int32_t setgid(gid_t gid)
{
	return syscall_setgid(gid);
}

_syscall0(getpgid);
static inline int32_t getpgid()
{
	return syscall_getpgid();
}

_syscall0(getppid);
static inline int32_t getppid()
{
	return syscall_getppid();
}

_syscall2(setpgid, pid_t, pid_t);
static inline int32_t setpgid(pid_t pid, pid_t pgid)
{
	return syscall_setpgid(pid, pgid);
}

_syscall0(getsid);
static inline int32_t getsid()
{
	return syscall_getsid();
}

_syscall0(setsid);
static inline int32_t setsid()
{
	return syscall_setsid();
}

_syscall1(posix_spawn, char *);
static inline int32_t posix_spawn(char *path)
{
	return syscall_posix_spawn(path);
}

_syscall2(getptsname, int32_t, char *);
static inline int32_t getptsname(int32_t fdm, char *ptsname)
{
	return syscall_getptsname(fdm, ptsname);
}

_syscall2(dprintf, enum debug_level, const char *);
static inline int32_t dprintf(enum debug_level level, const char *out)
{
	return syscall_dprintf(level, out);
}

_syscall2(dprintln, enum debug_level, const char *);
static inline int32_t dprintln(enum debug_level level, const char *out)
{
	return syscall_dprintln(level, out);
}

_syscall3(getdents, unsigned int, struct dirent *, unsigned int);
int getdents(unsigned int fd, struct dirent *dirent, unsigned int count);

int usleep(uint32_t usec);
int sleep(uint32_t sec);

int tcsetpgrp(int fd, pid_t pid);
pid_t tcgetpgrp(int fd);
int isatty(int fd);

int32_t shm_open(const char *name, int32_t flags, int32_t mode);

int getopt(int argc, char *const argv[], const char *optstring);
extern char *optarg;
extern int opterr, optind, optopt;

#endif

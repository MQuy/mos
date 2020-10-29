#ifndef _LIBC_UNISTD_H
#define _LIBC_UNISTD_H 1

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __USE_XOPEN2K8
/* POSIX Standard approved as ISO/IEC 9945-1 as of September 2008.  */
#define _POSIX_VERSION 200809L
#elif defined __USE_XOPEN2K
/* POSIX Standard approved as ISO/IEC 9945-1 as of December 2001.  */
#define _POSIX_VERSION 200112L
#elif defined __USE_POSIX199506
/* POSIX Standard approved as ISO/IEC 9945-1 as of June 1995.  */
#define _POSIX_VERSION 199506L
#elif defined __USE_POSIX199309
/* POSIX Standard approved as ISO/IEC 9945-1 as of September 1993.  */
#define _POSIX_VERSION 199309L
#else
/* POSIX Standard approved as ISO/IEC 9945-1 as of September 1990.  */
#define _POSIX_VERSION 199009L
#endif

#ifdef __USE_XOPEN2K8
#define __POSIX2_THIS_VERSION 200809L
/* The utilities on GNU systems also correspond to this version.  */
#elif defined __USE_XOPEN2K
/* The utilities on GNU systems also correspond to this version.  */
#define __POSIX2_THIS_VERSION 200112L
#elif defined __USE_POSIX199506
/* The utilities on GNU systems also correspond to this version.  */
#define __POSIX2_THIS_VERSION 199506L
#else
/* The utilities on GNU systems also correspond to this version.  */
#define __POSIX2_THIS_VERSION 199209L
#endif

/* The utilities on GNU systems also correspond to this version.  */
#define _POSIX2_VERSION __POSIX2_THIS_VERSIO

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

int fork();
void _exit(int code);
int read(int fd, char *buf, size_t size);
int write(int fd, const char *buf, size_t size);
int close(int fd);
int lseek(int fd, off_t offset, int whence);
int execve(const char *pathname, char *const argv[], char *const envp[]);
int dup2(int oldfd, int newfd);
int brk(intptr_t increment);
int sbrk(intptr_t increment);
int pipe(int *fildes);
int truncate(const char *name, off_t length);
int ftruncate(int fd, off_t length);
int getpid();
int getuid();
int setuid(uid_t uid);
int getegid();
int geteuid();
int getgid();
int setgid(gid_t gid);
int getpgid();
int getppid();
int setpgid(pid_t pid, pid_t pgid);
int getsid();
int setsid();
int posix_spawn(char *path);
int getptsname(int fdm, char *ptsname);
int getdents(unsigned int fd, struct dirent *dirent, unsigned int count);

int usleep(useconds_t usec);
int sleep(unsigned int);
int tcsetpgrp(int fd, pid_t pid);
pid_t tcgetpgrp(int fd);
int isatty(int fd);
int shm_open(const char *name, int flags, mode_t mode);
int getpagesize();

int getopt(int argc, char *const argv[], const char *optstring);
extern char *optarg;
extern int opterr, optind, optopt;

#endif

#ifndef _LIBC_UNISTD_H
#define _LIBC_UNISTD_H 1

#include <bits/confname.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* These may be used to determine what facilities are present at compile time.
   Their values can be obtained at run time from `sysconf'.  */

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

/* These are not #ifdef __USE_POSIX2 because they are
   in the theoretically application-owned namespace.  */

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
#define _POSIX2_VERSION __POSIX2_THIS_VERSION

/* This symbol was required until the 2001 edition of POSIX.  */
#define _POSIX2_C_VERSION __POSIX2_THIS_VERSION

/* If defined, the implementation supports the
   C Language Bindings Option.  */
#define _POSIX2_C_BIND __POSIX2_THIS_VERSION

/* If defined, the implementation supports the
   C Language Development Utilities Option.  */
#define _POSIX2_C_DEV __POSIX2_THIS_VERSION

/* If defined, the implementation supports the
   Software Development Utilities Option.  */
#define _POSIX2_SW_DEV __POSIX2_THIS_VERSION

/* If defined, the implementation supports the
   creation of locales with the localedef utility.  */
#define _POSIX2_LOCALEDEF __POSIX2_THIS_VERSION

#define R_OK 4 /* Test for read permission. */
#define W_OK 2 /* Test for write permission. */
#define X_OK 1 /* Test for execute permission. */
#define F_OK 0 /* Test for existence. */

#ifndef SEEK_SET
#define SEEK_SET 0 /* Seek from beginning of file.  */
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1 /* Seek from current position.  */
#endif

#ifndef SEEK_END
#define SEEK_END 2 /* Seek from end of file.  */
#endif

#define STDIN_FILENO 0	/* standard input file descriptor */
#define STDOUT_FILENO 1 /* standard output file descriptor */
#define STDERR_FILENO 2 /* standard error file descriptor */

#define _PC_2_SYMLINKS 1
#define _PC_ALLOC_SIZE_MIN 2
#define _PC_ASYNC_IO 3
#define _PC_CHOWN_RESTRICTED 4
#define _PC_FILESIZEBITS 5
#define _PC_LINK_MAX 6
#define _PC_MAX_CANON 7
#define _PC_MAX_INPUT 8
#define _PC_NAME_MAX 9
#define _PC_NO_TRUNC 10
#define _PC_PATH_MAX 11
#define _PC_PIPE_BUF 12
#define _PC_PRIO_IO 13
#define _PC_REC_INCR_XFER_SIZE 14
#define _PC_REC_MAX_XFER_SIZE 15
#define _PC_REC_MIN_XFER_SIZE 16
#define _PC_REC_XFER_ALIGN 17
#define _PC_SYMLINK_MAX 18
#define _PC_SYNC_IO 19
#define _PC_TIMESTAMP_RESOLUTION 20
#define _PC_VDISABLE 21

// FIXME MQ 2020-05-12 copy define constants from https://github.com/torvalds/linux/blob/master/arch/x86/entry/syscalls/syscall_32.tbl
#define __NR_exit 1
#define __NR_fork 2
#define __NR_read 3
#define __NR_write 4
#define __NR_open 5
#define __NR_close 6
#define __NR_waitpid 7
#define __NR_unlink 10
#define __NR_execve 11
#define __NR_chdir 12
#define __NR_time 13
#define __NR_mknod 14
#define __NR_chmod 15
#define __NR_brk 17
#define __NR_sbrk 18
#define __NR_lseek 19
#define __NR_getpid 20
#define __NR_setuid 23
#define __NR_getuid 24
#define __NR_alarm 27
#define __NR_access 33
#define __NR_kill 37
#define __NR_rename 38
#define __NR_dup 41
#define __NR_pipe 42
#define __NR_times 43
#define __NR_setgid 46
#define __NR_getgid 47
#define __NR_signal 48
#define __NR_geteuid 49
#define __NR_getegid 50
#define __NR_ioctl 54
#define __NR_fcntl 55
#define __NR_setpgid 57
#define __NR_umask 60
#define __NR_dup2 63
#define __NR_getppid 64
#define __NR_getpgrp 65
#define __NR_setsid 66
#define __NR_sigaction 67
#define __NR_sigsuspend 72
#define __NR_gettimeofday 78
#define __NR_mmap 90
#define __NR_munmap 91
#define __NR_truncate 92
#define __NR_ftruncate 93
#define __NR_fchmod 94
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
#define __NR_uname 122
#define __NR_sigprocmask 126
#define __NR_fchdir 133
#define __NR_getpgid 132
#define __NR_getdents 141
#define __NR_getsid 147
#define __NR_nanosleep 162
#define __NR_poll 168
#define __NR_getcwd 183
#define __NR_clock_gettime 265
#define __NR_mq_open 277
#define __NR_mq_close (__NR_mq_open + 1)
#define __NR_mq_unlink (__NR_mq_open + 2)
#define __NR_mq_send (__NR_mq_open + 3)
#define __NR_mq_receive (__NR_mq_open + 4)
#define __NR_waitid 284
#define __NR_mknodat 297
#define __NR_unlinkat 301
#define __NR_renameat 302
#define __NR_faccessat 307
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

#define SYSCALL_RETURN(expr) ({ int ret = expr; if (ret < 0) { return errno = -ret, -1; } return 0; })
#define SYSCALL_RETURN_ORIGINAL(expr) ({ int ret = expr; if (ret < 0) { return errno = -ret, -1; } return ret; })
#define SYSCALL_RETURN_POINTER(expr) ({ int ret = expr; if (ret < 0) { return errno = -ret, -1; } return ret; })

struct dirent;

int fork();
void _exit(int code);
int read(int fd, char *buf, size_t size);
int write(int fd, const char *buf, size_t size);
int close(int fd);
int lseek(int fd, off_t offset, int whence);
int execve(const char *pathname, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);
int execlp(const char *file, const char *arg0, ...);
int execl(const char *path, const char *arg0, ...);
int dup2(int oldfd, int newfd);
int brk(intptr_t increment);
int sbrk(intptr_t increment);
int pipe(int *fildes);
int truncate(const char *name, off_t length);
int ftruncate(int fd, off_t length);
char *getcwd(char *buf, size_t size);
int getpid();
int getuid();
int setuid(uid_t uid);
int getegid();
int geteuid();
int getgid();
int setgid(gid_t gid);
int getpgid(pid_t pid);
int getpgrp();
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
char *ttyname(int fildes);
int ttyname_r(int fildes, char *name, size_t namesize);
int isatty(int fd);
int shm_open(const char *name, int flags, mode_t mode);
int getpagesize();

int getopt(int argc, char *const argv[], const char *optstring);
extern char *optarg;
extern int opterr, optind, optopt;
extern char **environ;

int access(const char *path, int amode);
int faccessat(int fd, const char *path, int amode, int flag);

int link(const char *path1, const char *path2);
int linkat(int fd1, const char *path1, int fd2,
		   const char *path2, int flag);
int unlink(const char *path);
int unlinkat(int fd, const char *path, int flag);

int dup(int fildes);
int chdir(const char *path);
int fchdir(int fildes);

int fsync(int fd);
int fdatasync(int fd);

int chown(const char *path, uid_t owner, gid_t group);
int fchownat(int fd, const char *path, uid_t owner, gid_t group,
			 int flag);
int fchown(int fildes, uid_t owner, gid_t group);

long fpathconf(int fildes, int name);
long pathconf(const char *path, int name);

int mkdir(const char *path, mode_t mode);
int mkdirat(int fd, const char *path, mode_t mode);

int rmdir(const char *path);

long sysconf(int name);

#endif

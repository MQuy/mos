#ifndef _LIBC_FCNTL_H
#define _LIBC_FCNTL_H 1

#include <sys/types.h>

#define O_ACCMODE 0003
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 01000	/* not fcntl */
#define O_TRUNC 02000	/* not fcntl */
#define O_EXCL 04000	/* not fcntl */
#define O_NOCTTY 010000 /* not fcntl */

#define O_NONBLOCK 00004
#define O_APPEND 00010
#define O_NDELAY O_NONBLOCK
#define O_SYNC 040000
#define FASYNC 020000		/* fcntl, for BSD compatibility */
#define O_DIRECTORY 0100000 /* must be a directory */
#define O_NOFOLLOW 0200000	/* don't follow links */
#define O_LARGEFILE 0400000 /* will be set by the kernel on every open */
#define O_DIRECT 02000000	/* direct disk access - should check with OSF/1 */
#define O_NOATIME 04000000

#define F_DUPFD 0 /* dup */
#define F_GETFD 1 /* get close_on_exec */
#define F_SETFD 2 /* set/clear close_on_exec */
#define F_GETFL 3 /* get file->f_flags */
#define F_SETFL 4 /* set file->f_flags */
#define F_GETLK 5
#define F_SETLK 6
#define F_SETLKW 7
#define F_SETOWN 8	/* for sockets. */
#define F_GETOWN 9	/* for sockets. */
#define F_SETSIG 10 /* for sockets. */
#define F_GETSIG 11 /* for sockets. */

#define FD_CLOEXEC 1

#define AT_FDCWD -2

/* Flag values for faccessat2) et al. */
#define AT_EACCESS 1
#define AT_SYMLINK_NOFOLLOW 2
#define AT_SYMLINK_FOLLOW 4
#define AT_REMOVEDIR 8

int open(const char* path, int oflag, ...);
int fcntl(int fd, int cmd, ...);
int creat(const char* path, mode_t mode);

#endif

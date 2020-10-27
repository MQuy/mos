#ifndef _LIBC_FCNTL_H
#define _LIBC_FCNTL_H

#define SEEK_SET 0 /* Seek from beginning of file.  */
#define SEEK_CUR 1 /* Seek from current position.  */
#define SEEK_END 2 /* Seek from end of file.  */

// file
#define S_IFMT 00170000
#define S_IFIFO 0010000
#define S_IFSOCK 0140000
#define S_IFLNK 0120000
#define S_IFREG 0100000
#define S_IFBLK 0060000
#define S_IFDIR 0040000
#define S_IFCHR 0020000

#define S_ISLNK(m) (((m)&S_IFMT) == S_IFLNK)
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#define S_ISCHR(m) (((m)&S_IFMT) == S_IFCHR)
#define S_ISBLK(m) (((m)&S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m)&S_IFMT) == S_IFIFO)
#define S_ISSOCK(m) (((m)&S_IFMT) == S_IFSOCK)

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

#endif

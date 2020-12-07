#ifndef _LIBC_SYS_STAT_H
#define _LIBC_SYS_STAT_H 1

#include <sys/types.h>
#include <time.h>

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

#define S_IRWXU 0700
#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRWXG 070
#define S_IRGRP 040
#define S_IWGRP 020
#define S_IXGRP 010
#define S_IRWXO 07
#define S_IROTH 04
#define S_IWOTH 02
#define S_IXOTH 01
#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000

struct stat
{
	dev_t st_dev;		  /* ID of device containing file */
	ino_t st_ino;		  /* Inode number */
	mode_t st_mode;		  /* File type and mode */
	nlink_t st_nlink;	  /* Number of hard links */
	uid_t st_uid;		  /* User ID of owner */
	gid_t st_gid;		  /* Group ID of owner */
	dev_t st_rdev;		  /* Device ID (if special file) */
	off_t st_size;		  /* Total size, in bytes */
	blksize_t st_blksize; /* Block size for filesystem I/O */
	blkcnt_t st_blocks;	  /* Number of 512B blocks allocated */

	struct timespec st_atim; /* Time of last access */
	struct timespec st_mtim; /* Time of last modification */
	struct timespec st_ctim; /* Time of last status change */

	time_t st_atime; /* Time of last access.  */
	time_t st_mtime; /* Time of last modification.  */
	time_t st_ctime; /* Time of last status change.  */
};

int stat(const char *path, struct stat *buf);
int fstat(int fildes, struct stat *buf);
mode_t umask(mode_t cmask);
int chmod(const char *path, mode_t mode);
int fchmod(int fildes, mode_t mode);
int mkdir(const char *path, mode_t mode);
int mkdirat(int fd, const char *path, mode_t mode);
int mknod(const char *path, mode_t mode, dev_t dev);
int mknodat(int fd, const char *path, mode_t mode, dev_t dev);
int mkfifo(const char *path, mode_t mode);
int mkfifoat(int fd, const char *path, mode_t mode);

#endif

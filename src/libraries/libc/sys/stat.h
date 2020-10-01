#ifndef LIBC_SYS_STAT_H
#define LIBC_SYS_STAT_H

#include <sys/types.h>
#include <time.h>

struct stat
{
	unsigned long ino;
	dev_t dev;
	umode_t mode;
	unsigned int nlink;
	uid_t uid;
	gid_t gid;
	dev_t rdev;
	loff_t size;
	struct timespec atime;
	struct timespec mtime;
	struct timespec ctime;
	unsigned long blksize;
	unsigned long blocks;
};

#endif

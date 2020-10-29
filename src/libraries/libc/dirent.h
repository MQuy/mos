#ifndef _LIBC_DIRENT_H
#define _LIBC_DIRENT_H 1

#include <stddef.h>
#include <sys/types.h>

#define MAX_FILENAME_LENGTH 256

struct dirent
{
	ino_t d_ino;			 /* Inode number */
	off_t d_off;			 /* Not an offset; see below */
	unsigned short d_reclen; /* Length of this record */
	unsigned short d_type;	 /* Type of file; not supported
                                              by all filesystem types */
	char d_name[];			 /* Null-terminated filename */
};

struct __DIR
{
	struct dirent* entry;
	size_t size;
	int fd;
};

typedef struct __DIR DIR;

#endif

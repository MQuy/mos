#ifndef LIBC_DIRENT_H
#define LIBC_DIRENT_H

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

#endif

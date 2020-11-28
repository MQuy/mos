#ifndef _LIBC_FILE_H
#define _LIBC_FILE_H

#include <list.h>

struct __FILE
{
	int fd;
	int _flags;
	int pos;
	char *_IO_read_ptr, *_IO_read_base, *_IO_read_end;
	char *_IO_write_ptr, *_IO_write_base, *_IO_write_end;
	int bkup_chr;  // for ungetc
	int blksize;
	struct list_head sibling;
};

#endif

#ifndef _LIBC_FILE_H
#define _LIBC_FILE_H

#include <list.h>

struct __FILE
{
	int fd;
	int _flags;
	int _offset;
	char *_IO_read_ptr, *_IO_read_base, *_IO_read_end;
	char *_IO_write_ptr, *_IO_write_base, *_IO_write_end;
	int bkup_chr;  // for ungetc
	int blksize;
	struct list_head sibling;

	/* The following fields are used to support backing up and undo. */
	char *_IO_save_base;   /* Pointer to start of non-current get area. */
	char *_IO_backup_base; /* Pointer to first valid character of backup area */
	char *_IO_save_end;	   /* Pointer to end of non-current get area. */
};

#endif

#ifndef _LIBC_FILE_H
#define _LIBC_FILE_H

#include <list.h>

struct __FILE
{
	int fd;
	int flags;
	int pos;
	char *read_ptr, *read_base, *read_end;
	char *write_ptr, *write_base, *write_end;
	int bkup_chr;  // for ungetc
	int blksize;
	struct list_head sibling;
};

#endif

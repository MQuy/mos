#ifndef _LIBC_SYS_UTIME_H
#define _LIBC_SYS_UTIME_H

#include <sys/types.h>

struct utimbuf
{
	time_t actime;	/* Access time.  */
	time_t modtime; /* Modification time.  */
};

int utime(const char *path, const struct utimbuf *times);

#endif

#ifndef _LIBC_SYS_TIME_H
#define _LIBC_SYS_TIME_H 1

#include <sys/types.h>

struct timeval
{
	time_t tv_sec;
	suseconds_t tv_usec;
};

#endif

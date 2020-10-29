#ifndef _LIBC_SYS_TIME_H
#define _LIBC_SYS_TIME_H 1

#include <sys/types.h>

struct timeval
{
	time_t tv_sec;
	suseconds_t tv_usec;
};

struct timezone
{
	int tz_minuteswest;
	int tz_dsttime;
};

int gettimeofday(struct timeval *restrict, void *restrict);

#endif

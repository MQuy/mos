#ifndef _LIBC_TIME_H
#define _LIBC_TIME_H

#include <sys/types.h>

struct timespec
{
	time_t tv_sec;
	long tv_nsec;
};

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3
#define CLOCK_MONOTONIC_RAW 4
#define CLOCK_REALTIME_COARSE 5
#define CLOCK_MONOTONIC_COARSE 6
#define CLOCK_BOOTTIME 7
#define CLOCK_REALTIME_ALARM 8
#define CLOCK_BOOTTIME_ALARM 9

#define CLOCKS_PER_SEC 1000000

struct tm
{
	int tm_sec;	  /* Seconds (0-60) */
	int tm_min;	  /* Minutes (0-59) */
	int tm_hour;  /* Hours (0-23) */
	int tm_mday;  /* Day of the month (1-31) */
	int tm_mon;	  /* Month (0-11) */
	int tm_year;  /* Year - 1900 */
	int tm_wday;  /* Day of the week (0-6, Sunday = 0) */
	int tm_yday;  /* Day in the year (0-365, 1 Jan = 0) */
	int tm_isdst; /* Daylight saving time */
};

clock_t clock();
struct tm *localtime(const time_t *timer);
time_t mktime(struct tm *timeptr);

#endif

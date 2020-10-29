#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include <include/types.h>
#include <stdint.h>

struct timespec
{
	time_t tv_sec;
	long tv_nsec;
};

struct tms
{
	clock_t tms_utime;
	clock_t tms_stime;
	clock_t tms_cutime;
	clock_t tms_cstime;
};

struct timeval
{
	time_t tv_sec;
	suseconds_t tv_usec;
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

struct time
{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint16_t year;
};

void set_boot_seconds(uint64_t bs);
void set_current_time(uint16_t year, uint8_t month, uint8_t day,
					  uint8_t hour, uint8_t minute, uint8_t second);
uint32_t get_seconds(struct time *);
uint64_t get_milliseconds(struct time *t);
uint64_t get_milliseconds_since_epoch();
struct time *get_time(int32_t seconds);

#endif

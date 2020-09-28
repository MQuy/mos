#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include <stdint.h>

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

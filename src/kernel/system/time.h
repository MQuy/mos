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

uint32_t get_seconds(struct time *);
struct time *get_time(int32_t seconds);

#endif
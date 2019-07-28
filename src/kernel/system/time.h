#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include <stdint.h>

typedef struct time
{
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint16_t year;
} time;

uint32_t get_seconds(time *);
time *get_time(uint32_t seconds);

#endif
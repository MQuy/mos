#include <kernel/cpu/hal.h>
#include <kernel/memory/vmm.h>
#include "time.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

uint8_t get_update_flag()
{
  outportb(CMOS_ADDRESS, 0x0A);
  return inportb(CMOS_DATA) & 0x80;
}

uint8_t get_rtc_register(uint32_t reg)
{
  outportb(CMOS_ADDRESS, reg);
  return inportb(CMOS_DATA);
}

time *get_current_time()
{
  uint8_t second, minute, hour, day, month, year;
  time *t = kcalloc(1, sizeof(struct time));

  while (get_update_flag())
    ;
  second = get_rtc_register(0x00);
  minute = get_rtc_register(0x02);
  hour = get_rtc_register(0x04);
  day = get_rtc_register(0x07);
  month = get_rtc_register(0x08);
  year = get_rtc_register(0x09);

  uint8_t registerB = get_rtc_register(0x0B);
  // Convert BCD to binary values if necessary
  if (!(registerB & 0x04))
  {
    second = (second & 0x0F) + ((second / 16) * 10);
    minute = (minute & 0x0F) + ((minute / 16) * 10);
    hour = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
    day = (day & 0x0F) + ((day / 16) * 10);
    month = (month & 0x0F) + ((month / 16) * 10);
    year = (year & 0x0F) + ((year / 16) * 10);
  }

  // Convert 12 hour clock to 24 hour clock if necessary
  if (!(registerB & 0x02) && (hour & 0x80))
  {
    hour = ((hour & 0x7F) + 12) % 24;
  }

  t->second = second;
  t->minute = minute;
  t->hour = hour;
  t->day = day;
  t->month = month;
  t->year = (year >= 70 ? 1900 : 2000) + year;

  return t;
}

time *get_time_from_seconds(uint32_t seconds)
{
  time *t = kcalloc(1, sizeof(struct time));
  uint32_t days = seconds / (24 * 3600);

  // NOTE: MQ 2019-07-25 According to this paper http://howardhinnant.github.io/date_algorithms.html#civil_from_days
  days += 719468;
  uint32_t era = (days >= 0 ? days : days - 146096) / 146097;
  uint32_t doe = days - era * 146097;                                   // [0, 146096]
  uint32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365; // [0, 399]
  uint32_t y = yoe + era * 400;
  uint32_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100); // [0, 365]
  uint32_t mp = (5 * doy + 2) / 153;                      // [0, 11]
  uint32_t d = doy - (153 * mp + 2) / 5 + 1;              // [1, 31]
  uint32_t m = mp + (mp < 10 ? 3 : -9);                   // [1, 12]

  t->year = y + (m <= 2);
  t->month = m;
  t->day = d;
  t->hour = (seconds % (24 * 3600)) / 3600;
  t->minute = (seconds % (60 * 60)) / 60;
  t->second = seconds % 60;

  return t;
}

// NOTE: MQ 2019-07-25 According to this paper http://howardhinnant.github.io/date_algorithms.html#days_from_civil
uint32_t get_days(time *t)
{
  int32_t year = t->year;
  uint32_t month = t->month;
  uint32_t day = t->day;

  year -= month <= 2;

  uint32_t era = (year >= 0 ? year : year - 399) / 400;
  uint32_t yoe = (year - era * 400);                                       // [0, 399]
  uint32_t doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1; // [0, 365]
  uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;                    // [0, 146096]
  return era * 146097 + (doe)-719468;
}

uint32_t get_seconds(time *t)
{
  if (t == NULL)
    t = get_current_time();

  return get_days(t) * 24 * 3600 + t->hour * 3600 + t->minute * 60 + t->second;
}

time *get_time(uint32_t seconds)
{
  return seconds == NULL ? get_current_time() : get_time_from_seconds(seconds);
}
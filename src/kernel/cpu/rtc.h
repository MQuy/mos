#ifndef CPU_RTC_H
#define CPU_RTC_H

#include <stdint.h>

void rtc_init();
void rtc_get_datetime(uint16_t *year, uint8_t *month, uint8_t *day,
					  uint8_t *hour, uint8_t *minute, uint8_t *second);

#endif

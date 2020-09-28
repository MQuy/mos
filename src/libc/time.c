#include "time.h"

#include <libc/stdlib.h>

struct tm *localtime(const time_t *timer)
{
	int seconds = *timer;
	struct tm *t = calloc(1, sizeof(struct tm));
	int days = seconds / (24 * 3600);

	days += 719468;
	unsigned int era = (days >= 0 ? days : days - 146096) / 146097;
	unsigned int doe = days - era * 146097;									   // [0, 146096]
	unsigned int yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;  // [0, 399]
	unsigned int y = yoe + era * 400;
	unsigned int doy = doe - (365 * yoe + yoe / 4 - yoe / 100);	 // [0, 365]
	unsigned int mp = (5 * doy + 2) / 153;						 // [0, 11]
	unsigned int d = doy - (153 * mp + 2) / 5 + 1;				 // [1, 31]
	unsigned int m = mp + (mp < 10 ? 3 : -9);					 // [1, 12]

	t->tm_year = y + (m <= 2);
	t->tm_mon = m;
	t->tm_mday = d;
	t->tm_hour = (seconds % (24 * 3600)) / 3600;
	t->tm_min = (seconds % (60 * 60)) / 60;
	t->tm_sec = seconds % 60;

	return t;
}

time_t mktime(struct tm *timeptr)
{
	int year = timeptr->tm_year;
	unsigned int month = timeptr->tm_mon;
	unsigned int day = timeptr->tm_mday;

	year -= month <= 2;

	unsigned int era = (year >= 0 ? year : year - 399) / 400;
	unsigned int yoe = (year - era * 400);										  // [0, 399]
	unsigned int doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;  // [0, 365]
	unsigned int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;					  // [0, 146096]
	return era * 146097 + (doe)-719468;
}

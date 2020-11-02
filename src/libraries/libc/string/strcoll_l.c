#include <string.h>

int strcoll_l(const char* s1, const char* s2, locale_t locale)
{
	(void)locale;
	return strcoll(s1, s2);
}

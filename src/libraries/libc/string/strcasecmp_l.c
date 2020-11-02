#include <strings.h>

int strcasecmp_l(const char *s1, const char *s2, locale_t locale)
{
	(void)locale;
	return strcasecmp(s1, s2);
}

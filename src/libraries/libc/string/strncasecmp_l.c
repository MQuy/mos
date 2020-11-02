#include <strings.h>

int strncasecmp_l(const char *s1, const char *s2, size_t n, locale_t locale)
{
	return strncasecmp(s1, s2, n);
}

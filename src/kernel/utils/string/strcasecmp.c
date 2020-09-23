#include <kernel/utils/string.h>

int strcasecmp(const char *s1, const char *s2)
{
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;
	int result;

	if (p1 == p2)
		return 0;

	while ((result = tolower(*p1) - tolower(*p2++)) == 0)
		if (*p1++ == '\0')
			break;

	return result;
}

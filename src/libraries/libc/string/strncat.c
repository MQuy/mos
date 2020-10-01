#include <string.h>

char *strncat(char *s1, const char *s2, size_t n)
{
	char *s = s1;

	/* Find the end of S1.  */
	s1 += strlen(s1);

	size_t ss = strnlen(s2, n);

	s1[ss] = '\0';
	memcpy(s1, s2, ss);

	return s;
}

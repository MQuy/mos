#include <libc/string.h>

char *strncpy(char *s1, const char *s2, size_t n)
{
	size_t size = strnlen(s2, n);
	if (size != n)
		memset(s1 + size, '\0', n - size);
	return memcpy(s1, s2, size);
}

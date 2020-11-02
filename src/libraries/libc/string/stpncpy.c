#include <string.h>

char *stpncpy(char *dest, const char *src, size_t n)
{
	size_t size = strnlen(src, n);
	memcpy(dest, src, size);
	dest += size;
	if (size == n)
		return dest;
	return memset(dest, '\0', n - size);
}

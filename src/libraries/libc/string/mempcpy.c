#include <string.h>

void *mempcpy(void *dest, const void *src, size_t len)
{
	return memcpy(dest, src, len) + len;
}

#include <string.h>

char *stpcpy(char *dest, const char *src)
{
	size_t len = strlen(src);
	return memcpy(dest, src, len + 1) + len;
}

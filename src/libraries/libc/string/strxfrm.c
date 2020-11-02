#include <string.h>

size_t strxfrm(char* dest, const char* src, size_t n)
{
	size_t srclen = strlen(src);
	strncpy(dest, src, n);
	return srclen;
}

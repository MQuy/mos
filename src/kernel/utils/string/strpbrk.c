#include <utils/string.h>

char *strpbrk(const char *s, const char *accept)
{
	s += strcspn(s, accept);
	return *s ? (char *)s : NULL;
}

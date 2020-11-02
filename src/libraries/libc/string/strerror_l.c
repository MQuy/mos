#include <string.h>

char* strerror_l(int errnum, locale_t locale)
{
	(void)locale;
	return (char*)strerror(errnum);
}

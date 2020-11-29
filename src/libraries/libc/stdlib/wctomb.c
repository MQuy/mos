#include <assert.h>
#include <stdlib.h>

int wctomb(char *s, wchar_t wchar)
{
	assert_not_reached();
	__builtin_unreachable();
}

size_t wcstombs(char *restrict s, const wchar_t *restrict pwcs,
				size_t n)
{
	assert_not_reached();
	__builtin_unreachable();
}

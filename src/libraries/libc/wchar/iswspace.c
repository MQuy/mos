#include <wctype.h>

int iswspace(wint_t c)
{
	return c == L'\t' || c == L'\n' || c == L'\v' ||
		   c == L'\f' || c == L'\r' || c == L' ';
}

#include <inttypes.h>

imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom)
{
	imaxdiv_t div;
	div.quot = numer / denom;
	div.rem = numer % denom;
	return div;
}

#include <stdlib.h>

div_t div(int numer, int denom)
{
	div_t ret;
	ret.quot = numer / denom;
	ret.rem = numer % denom;
	return ret;
}

ldiv_t ldiv(long numer, long denom)
{
	ldiv_t ret;
	ret.quot = numer / denom;
	ret.rem = numer % denom;
	return ret;
}

lldiv_t lldiv(long long numer, long long denom)
{
	lldiv_t ret;
	ret.quot = numer / denom;
	ret.rem = numer % denom;
	return ret;
}

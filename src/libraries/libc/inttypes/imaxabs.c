#include <inttypes.h>

intmax_t imaxabs(intmax_t val)
{
	return val < 0 ? -val : val;
}

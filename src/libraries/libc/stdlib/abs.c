#include <stdlib.h>

int abs(int num)
{
	return num < 0 ? -num : num;
}

long int labs(long int val)
{
	return val < 0 ? -val : val;
}

long long int llabs(long long int val)
{
	return val < 0 ? -val : val;
}

#ifndef LIBC_MATH_H
#define LIBC_MATH_H

#include <stdint.h>

// NOTE: MQ 2019-11-28
// remember to wrap parameters parenthesis when defining macro because each parameter can be an expression
// for example:
// -> x = 60 - 0, y = 10
// if we don't wrap parameters in parenthesis, we get an unexpected behavior
// -> 60 - 0 / 10 + 60 - 0 % 10 > 10
#define div_ceil(x, y) ({__auto_type _x = (x); __auto_type _y = (y); _x / _y + (_x % _y > 0); })
#define round(x, y) ({__auto_type _x = (x); __auto_type _y = (y); (_x / _y) * _y ; })
#define abs(x) ({__auto_type _x = (x); _x >=0 ? _x : -_x; })

uint32_t rand();
uint32_t srand(uint32_t seed);

#endif

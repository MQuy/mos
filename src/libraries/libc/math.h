#ifndef _LIBC_MATH_H
#define _LIBC_MATH_H 1

#include <stdint.h>

#define log2(X) ((unsigned)(8 * sizeof(unsigned long long) - __builtin_clzll((X)) - 1))

/*
 * min()/max() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x, y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x < _y ? _x : _y; })

#define max(x, y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })

/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max at all, of course.
 */
#define min_t(type, x, y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#define max_t(type, x, y) \
	({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })

// NOTE: MQ 2019-11-28
// remember to wrap parameters parenthesis when defining macro because each parameter can be an expression
// for example:
// -> x = 60 - 0, y = 10
// if we don't wrap parameters in parenthesis, we get an unexpected behavior
// -> 60 - 0 / 10 + 60 - 0 % 10 > 10
#define div_ceil(x, y) ((x) / (y) + ((x) % (y) > 0))

#endif

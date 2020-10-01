#ifndef LIBC_MATH_H
#define LIBC_MATH_H

#include <stdint.h>

/* Align a value by rounding down to closest size.
   e.g. Using size of 4096, we get this behavior:
	{4095, 4096, 4097} = {0, 4096, 4096}.  */
#define ALIGN_DOWN(base, size) ((base) & -((__typeof__(base))(size)))

/* Align a value by rounding up to closest size.
   e.g. Using size of 4096, we get this behavior:
	{4095, 4096, 4097} = {4096, 4096, 8192}.
  Note: The size argument has side effects (expanded multiple times).  */
#define ALIGN_UP(base, size) ALIGN_DOWN((base) + (size)-1, (size))

/* Same as ALIGN_DOWN(), but automatically casts when base is a pointer.  */
#define PTR_ALIGN_DOWN(base, size) \
	((__typeof__(base))ALIGN_DOWN((uintptr_t)(base), (size)))

/* Same as ALIGN_UP(), but automatically casts when base is a pointer.  */
#define PTR_ALIGN_UP(base, size) \
	((__typeof__(base))ALIGN_UP((uintptr_t)(base), (size)))

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
#define div_ceil(x, y) ({__auto_type _x = (x); __auto_type _y = (y); _x / _y + (_x % _y > 0); })
#define round(x, y) ({__auto_type _x = (x); __auto_type _y = (y); (_x / _y) * _y ; })
#define abs(x) ({__auto_type _x = (x); _x >=0 ? _x : -_x; })

uint32_t rand();
uint32_t srand(uint32_t seed);

#endif

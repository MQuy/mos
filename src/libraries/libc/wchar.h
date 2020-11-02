#ifndef _LIBC_WCHAR_H
#define _LIBC_WCHAR_H 1

#include <sys/types.h>

#ifndef __wint_t_defined
#define __wint_t_defined
typedef __wint_t wint_t;
#endif

int iswspace(wint_t c);

#endif

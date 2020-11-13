#ifndef _LIBC_WCHAR_H
#define _LIBC_WCHAR_H 1

#include <sys/types.h>

#ifndef __FILE_defined
#define __FILE_defined
#include <FILE.h>
typedef struct __FILE FILE;
#endif

#ifndef __locale_t_defined
#define __locale_t_defined
typedef __locale_t locale_t;
#endif

#ifndef __size_t_defined
#define __size_t_defined
#define __need_size_t
#include <stddef.h>
#endif

#ifndef __wchar_t_defined
#define __wchar_t_defined
#define __need_wchar_t
#include <stddef.h>
#endif

#ifndef __wint_t_defined
#define __wint_t_defined
typedef __wint_t wint_t;
#endif

#ifndef __wctype_t_defined
#define __wctype_t_defined
typedef int (*wctype_t)(wint_t);
#endif

#define WCHAR_MAX __WCHAR_MAX__
#define WCHAR_MIN __WCHAR_MIN__

#ifndef WEOF
#define WEOF (0xffffffffu)
#endif

#ifndef NULL
#define __need_NULL
#include <stddef.h>
#endif

typedef struct
{
	unsigned short count;
	unsigned short length;
	wint_t wch;
} mbstate_t;

size_t wcslen(const wchar_t*);
wchar_t* wcscpy(wchar_t*, const wchar_t*);
wchar_t* wcsncpy(wchar_t*, const wchar_t*, size_t);
int wcscmp(const wchar_t*, const wchar_t*);
wchar_t* wcschr(const wchar_t*, int);
const wchar_t* wcsrchr(const wchar_t*, wchar_t);
wchar_t* wcscat(wchar_t*, const wchar_t*);
wchar_t* wcstok(wchar_t*, const wchar_t*, wchar_t**);
wchar_t* wcsncat(wchar_t*, const wchar_t*, size_t);
size_t mbrtowc(wchar_t* pwc, const char* s, size_t n, mbstate_t* ps);

#endif

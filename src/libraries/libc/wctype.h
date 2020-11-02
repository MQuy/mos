#ifndef _LIBC_WCTYPE_H
#define _LIBC_WCTYPE_H 1

#include <sys/types.h>

#ifndef __wint_t_defined
#define __wint_t_defined
typedef __wint_t wint_t;
#endif

typedef unsigned int wctrans_t;

#ifndef __wctype_t_defined
#define __wctype_t_defined
typedef int (*wctype_t)(wint_t);
#endif

#ifndef __locale_t_defined
#define __locale_t_defined
typedef __locale_t locale_t;
#endif

#ifndef WEOF
#define WEOF (0xffffffffu)
#endif

int iswalnum(wint_t);
int iswalpha(wint_t);
int iswblank(wint_t);
int iswcntrl(wint_t);
int iswctype(wint_t, wctype_t);
int iswdigit(wint_t);
int iswgraph(wint_t);
int iswlower(wint_t);
int iswprint(wint_t);
int iswpunct(wint_t);
int iswspace(wint_t);
int iswupper(wint_t);
int iswxdigit(wint_t);
wint_t towlower(wint_t);
wint_t towupper(wint_t);
wctype_t wctype(const char *);

#endif

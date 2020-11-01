#ifndef _LIBC_CTYPE_H
#define _LIBC_CTYPE_H 1

#include <sys/types.h>

#ifndef locale_t_defined
#define locale_t_defined
typedef __locale_t locale_t;
#endif

int isalnum(int);
int isalpha(int);
int isascii(int);
int isblank(int);
int iscntrl(int);
int isdigit(int);
int isgraph(int);
int islower(int);
int isprint(int);
int ispunct(int);
int isspace(int);
int isupper(int);
int isxdigit(int);
int toascii(int);
int tolower(int);
int toupper(int);

#endif

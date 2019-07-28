#ifndef INCLUDE_CTYPE_H
#define INCLUDE_CTYPE_H

typedef unsigned short umode_t;
typedef long long loff_t;
typedef unsigned long long ino_t;
typedef unsigned short mode_t;
typedef long ssize_t;
typedef unsigned long sector_t;
typedef unsigned short uid_t;
typedef unsigned short gid_t;

#define isspace(c) ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define isupper(c) ((c) >= 'A' && (c) <= 'Z')
#define islower(c) ((c) >= 'a' && (c) <= 'z')
#define isalpha(c) (isupper(c) || islower(c))
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define isxdigit(c) (isdigit(c) || ((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))
#define isprint(c) ((c) >= ' ' && (c) <= '~')
#define toupper(c) ((c)-0x20 * (((c) >= 'a') && ((c) <= 'z')))
#define tolower(c) ((c) + 0x20 * (((c) >= 'A') && ((c) <= 'Z')))
#define isascii(c) ((unsigned)(c) <= 0x7F)
#define toascii(c) ((unsigned)(c)&0x7F)

struct list_head
{
  struct list_head *next, *prev;
};

struct timespec
{
  long tv_sec;
  long tv_nsec;
};

#endif

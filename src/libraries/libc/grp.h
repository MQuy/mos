#ifndef _LIBC_GRP_H
#define _LIBC_GRP_H 1

#include <stddef.h>
#include <sys/types.h>

struct group
{
	gid_t gr_gid;
	char **gr_mem;
	char *gr_name;
	char *gr_passwd;
};

void endgrent(void);
struct group *getgrent(void);
struct group *getgrgid(gid_t);
int getgrgid_r(gid_t, struct group *, char *,
			   size_t, struct group **);
struct group *getgrnam(const char *);
int getgrnam_r(const char *, struct group *, char *,
			   size_t, struct group **);
void setgrent(void);

#endif

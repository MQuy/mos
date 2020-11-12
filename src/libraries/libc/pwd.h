#ifndef _LIBC_PWD_H
#define _LIBC_PWD_H 1

#include <stddef.h>
#include <sys/types.h>

#ifndef __FILE_defined
#define __FILE_defined
#include <FILE.h>
typedef struct __FILE FILE;
#endif

struct passwd
{
	char *pw_name;	 // User's login name.
	uid_t pw_uid;	 // Numerical user ID.
	gid_t pw_gid;	 // Numerical group ID.
	char *pw_dir;	 // Initial working directory.
	char *pw_shell;	 // Program to use as shell.
	char *pw_gecos;
	char *pw_passwd;
};

FILE *openpw(void);
struct passwd *getpwuid(uid_t uid);
int getpwuid_r(uid_t uid, struct passwd *ret, char *buf, size_t buflen, struct passwd **ret_ptr);
int fgetpwent_r(FILE *fp, struct passwd *result, char *buf, size_t buf_len, struct passwd **result_pointer);

#endif

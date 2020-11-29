#ifndef _LIBC_MNTENT_H
#define _LIBC_MNTENT_H 1

#include <paths.h>

#ifndef __FILE_defined
#define __FILE_defined
#include <FILE.h>
typedef struct __FILE FILE;
#endif

/* File listing canonical interesting mount points.  */
#define MNTTAB _PATH_MNTTAB /* Deprecated alias.  */

/* File listing currently active mount points.  */
#define MOUNTED _PATH_MOUNTED /* Deprecated alias.  */

struct mntent
{
	char *mnt_fsname; /* Device or server for filesystem.  */
	char *mnt_dir;	  /* Directory mounted on.  */
	char *mnt_type;	  /* Type of filesystem: ufs, nfs, etc.  */
	char *mnt_opts;	  /* Comma-separated options for fs.  */
	int mnt_freq;	  /* Dump frequency (in days).  */
	int mnt_passno;	  /* Pass number for `fsck'.  */
};

FILE *setmntent(const char *filename, const char *type);
struct mntent *getmntent(FILE *stream);
int addmntent(FILE *stream, const struct mntent *mnt);
int endmntent(FILE *streamp);
char *hasmntopt(const struct mntent *mnt, const char *opt);

#endif

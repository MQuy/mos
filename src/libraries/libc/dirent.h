#ifndef _LIBC_DIRENT_H
#define _LIBC_DIRENT_H 1

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#define MAX_FILENAME_LENGTH 256

enum
{
	DT_UNKNOWN = 0,
#define DT_UNKNOWN DT_UNKNOWN
	DT_FIFO = 1,
#define DT_FIFO DT_FIFO
	DT_CHR = 2,
#define DT_CHR DT_CHR
	DT_DIR = 4,
#define DT_DIR DT_DIR
	DT_BLK = 6,
#define DT_BLK DT_BLK
	DT_REG = 8,
#define DT_REG DT_REG
	DT_LNK = 10,
#define DT_LNK DT_LNK
	DT_SOCK = 12,
#define DT_SOCK DT_SOCK
	DT_WHT = 14
#define DT_WHT DT_WHT
};

struct dirent
{
	ino_t d_ino;			 /* Inode number */
	off_t d_off;			 /* Not an offset; see below */
	unsigned short d_reclen; /* Length of this record */
	unsigned short d_type;	 /* Type of file; not supported
                                              by all filesystem types */
	char d_name[];			 /* Null-terminated filename */
};

struct __DIR
{
	int fd;
	bool owned_fd;
	int len;	/* buffer length */
	int size;	/* amount of data in buffer */
	char *buf;	/* buffer */
	loff_t pos; /* position in buffer */
};

typedef struct __DIR DIR;

int closedir(DIR *dirp);
int dirfd(DIR *dirp);
DIR *opendir(const char *name);
DIR *fdopendir(int fd);
struct dirent *readdir(DIR *dirp);
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
void rewinddir(DIR *dirp);
void seekdir(DIR *dirp, long int loc);
long int telldir(DIR *dirp);

#endif

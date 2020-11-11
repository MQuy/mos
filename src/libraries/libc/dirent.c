#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

DIR *opendir(const char *name)
{
	int fd = open(name, O_RDONLY);
	DIR *dirp = fdopendir(fd);
	dirp->owned_fd = true;
	return dirp;
}

DIR *fdopendir(int fd)
{
	DIR *dirp = calloc(1, sizeof(sizeof(DIR)));
	dirp->fd = fd;
	dirp->owned_fd = false;
	dirp->size = 0;
	dirp->pos = 0;

	struct stat stat;
	fstat(fd, &stat);
	dirp->len = stat.st_blksize;
	dirp->buf = calloc(dirp->len, sizeof(char));

	return dirp;
}

struct dirent *readdir(DIR *dirp)
{
	if (dirp->fd < 0)
		return errno = EBADF, NULL;

	if (!dirp->size || dirp->pos == dirp->len)
	{
		memset(dirp->buf, 0, dirp->len);
		dirp->size = getdents(dirp->fd, dirp->buf, dirp->len);
		dirp->pos = 0;
	}
	if (!dirp->size)
		return NULL;

	struct dirent *entry = dirp->buf + dirp->pos;
	dirp->pos += entry->d_reclen;
	return entry;
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result)
{
	if (dirp->fd < 0)
		return errno = EBADF, NULL;

	struct dirent *pdir = readdir(dirp);
	memcpy(entry, pdir, min(entry->d_reclen, pdir->d_reclen));

	*result = pdir ? entry : NULL;
	return 0;
}

int closedir(DIR *dirp)
{
	if (dirp->fd < 0)
		return errno = EBADF, -1;

	if (dirp->owned_fd)
		close(dirp->fd);
	dirp->fd = -1;
	return 0;
}

void rewinddir(DIR *dirp)
{
	lseek(dirp->fd, 0, SEEK_SET);
	dirp->pos = 0;
	dirp->size = 0;
}

long telldir(DIR *dirp)
{
	if (dirp->fd < 0)
		return errno = EBADF, -1;

	return dirp->pos;
}

void seekdir(DIR *dirp, long loc)
{
	dirp->pos = loc;
}

int dirfd(DIR *dirp)
{
	if (dirp->fd < 0)
		return errno = EINVAL, -1;

	return dirp->fd;
}

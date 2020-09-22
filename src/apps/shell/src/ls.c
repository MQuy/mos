#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>

#include "command_line.h"

int ls(char *path)
{
	int fd = open(path, O_RDONLY, 0);

	struct stat *stat = calloc(1, sizeof(struct stat));
	fstat(fd, stat);

	struct dirent *dirent = calloc(stat->size, sizeof(char));
	int count = getdents(fd, dirent, stat->size);
	char filename[MAX_FILENAME_LENGTH];
	int filename_len;

	for (int pos = 0; pos < count;)
	{
		struct dirent *d = (struct dirent *)((char *)dirent + pos);

		filename_len = d->d_reclen - sizeof(struct dirent);
		memcpy(filename, d->d_name, filename_len);
		filename[filename_len] = 0;

		write(1, "\n", 1);
		write(1, filename, filename_len);

		pos += d->d_reclen;
	}

	return 0;
}

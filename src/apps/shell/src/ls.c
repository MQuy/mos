#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "command_line.h"

// TODO: 2020-09-24 Replace getidents with opendir and readir to get rid of fixed size
#define MAX_FOLDER_SIZE 4096

int ls(char *path)
{
	int fd = open(path, O_RDONLY, 0);

	struct stat *stat = calloc(1, sizeof(struct stat));
	fstat(fd, stat);

	int size = stat->st_size ? stat->st_size : MAX_FOLDER_SIZE;
	struct dirent *dirent = calloc(size, sizeof(char));
	int count = getdents(fd, dirent, size);
	char filename[MAX_FILENAME_LENGTH];
	int filename_len;

	for (int pos = 0; pos < count;)
	{
		struct dirent *d = (struct dirent *)((char *)dirent + pos);

		filename_len = d->d_reclen - sizeof(struct dirent) - 1;	 // - 1 because d->d_name contains null-terminated character
		memcpy(filename, d->d_name, filename_len);
		filename[filename_len] = 0;

		write(1, filename, filename_len);

		pos += d->d_reclen;
		if (pos < count)
			write(1, "\n", 1);
	}
	write(1, "\n", 1);

	return 0;
}

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char ttyname_buf[TTY_NAME_MAX + 1] = _PATH_DEV;

char *ttyname(int fd)
{
	int ret = ttyname_r(fd, ttyname_buf, sizeof(ttyname_buf));
	if (ret)
		return NULL;
	return ttyname_buf;
}

int ttyname_r(int fd, char *name, size_t namesize)
{
	if (!isatty(fd))
		return ENOTTY;

	struct stat sb;
	if (fstat(fd, &sb) || !S_ISCHR(sb.st_mode))
		return ENOTTY;

	char entry_path[PATH_MAX];
	int path_dev_length = sizeof(_PATH_DEV) - 1;
	memcpy(entry_path, _PATH_DEV, path_dev_length);

	DIR *dirp = opendir(_PATH_DEV);
	struct dirent *entry;
	struct stat esb;
	while ((entry = readdir(dirp)))
	{
		if (entry->d_ino != sb.st_ino)
			continue;

		strcpy(entry_path + path_dev_length, entry->d_name);
		if (stat(entry_path, &esb) || esb.st_dev != sb.st_dev || esb.st_ino != sb.st_ino)
			continue;

		closedir(dirp);
		int len = strlen(entry_path);
		if (len < namesize)
		{
			memcpy(name, entry_path, len);
			return 0;
		}
		else
			return ERANGE;
	}
	closedir(dirp);
	return EBADF;
}

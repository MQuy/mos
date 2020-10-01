#include <shared/mman.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../shell.h"
#include "command_line.h"

int cd(struct command_line *cmd)
{
	int fd = shm_open("shell", 0, 0);
	struct shell *ishell = (struct shell *)mmap(NULL, sizeof(struct shell), PROT_WRITE | PROT_READ, MAP_SHARED, fd);

	char *path = cmd->args[1];
	if (!path)
	{
		strcpy(ishell->cwd, "/");
		return 0;
	}
	else if (!strcmp(path, ".."))
	{
		int pend = 0;
		for (int i = 0, len = strlen(ishell->cwd); i < len; ++i)
		{
			if (ishell->cwd[i] == '/')
				pend = i;
		}
		pend = max(pend, 1);
		ishell->cwd[pend] = 0;
		return 0;
	}
	else if (!strcmp(path, "."))
		return 0;

	char *fpath = NULL;
	if (path[0] != '/')
	{
		int cwd_len = strlen(ishell->cwd);
		int path_len = strlen(path);
		bool cwd_blackslash_ending = ishell->cwd[cwd_len - 1] == '/';
		int fpath_len = cwd_len + path_len + (cwd_blackslash_ending ? 1 : 2);
		fpath = calloc(fpath_len, sizeof(char));
		for (int i = 0; i < fpath_len; ++i)
		{
			if (i < cwd_len)
				fpath[i] = ishell->cwd[i];
			else if (cwd_blackslash_ending)
				fpath[i] = path[i - cwd_len];
			else if (i == cwd_len)
				fpath[i] = '/';
			else
				fpath[i] = path[i - cwd_len - 1];
		}
	}
	else
		fpath = strdup(path);

	strcpy(ishell->cwd, fpath);
	free(fpath);

	return 0;
}

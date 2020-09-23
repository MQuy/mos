#include "shell.h"

#include <include/ioctls.h>
#include <include/mman.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>
#include <stdbool.h>

#include "src/command_line.h"

struct shell *ishell;

void init_shell()
{
	int32_t fd = shm_open("shell", 0, 0);
	ftruncate(fd, sizeof(struct shell));
	ishell = (struct shell *)mmap(NULL, sizeof(struct shell), PROT_WRITE | PROT_READ, MAP_SHARED, fd);
	memcpy(ishell->cwd, ROOT_PATH, sizeof(ROOT_PATH) - 1);
}

void print_prefix(char *cwd)
{
	char *dir = NULL;
	char *name = NULL;
	if (!strcmp(cwd, "/"))
		name = cwd;
	else
		strlsplat(cwd, strliof(cwd, "/"), &dir, &name);

	char prefix[200] = {0};
	sprintf(prefix, "$ %s ", name);

	if (dir)
		free(dir);
	if (name && name != cwd)
		free(name);

	write(1, prefix, strlen(prefix));
}

int main()
{
	init_shell();
	write(1, "\21", 1);
	print_prefix(ishell->cwd);

	char line[CHARACTERS_PER_LINE];
	struct command_line *cmd = calloc(1, sizeof(struct command_line));
	struct infop *infop = calloc(1, sizeof(struct infop));
	int ret;
	while (true)
	{
		memset(line, 0, sizeof(line));
		ret = read(0, line, sizeof(line));
		if (ret < 0)
			continue;

		ret = parse_text(line, ret, cmd);
		if (ret < 0)
			continue;

		int fd = fork();
		if (!fd)
		{
			if (cmd->is_builtin)
			{
				setpgid(0, 0);
				tcsetpgrp(1, getpid());
				if (!strcmp(cmd->program, "pwd"))
					pwd(ishell->cwd);
				else if (!strcmp(cmd->program, "ls"))
					ls(ishell->cwd);
				else if (!strcmp(cmd->program, "cd"))
					cd(cmd);
				else
					write(1, "\ncommand not found", 18);
				exit(0);
			}
		}
		else
		{
			waitid(P_PID, fd, infop, WEXITED);
			write(1, "\n\21", 2);
			print_prefix(ishell->cwd);
		}
	}
}

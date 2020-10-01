#include "shell.h"

#include <shared/ioctls.h>
#include <shared/mman.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

void execute_program(char *name, struct command_line *cmd)
{
	char *path = calloc(MAX_PATH_LENGTH, sizeof(char));
	if (name[0] != '/')
	{
		strcat(path, "/bin/");
		strcat(path, name);
	}
	else
		strcpy(path, name);
	execve(path, cmd->args, NULL);
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
			setpgid(0, 0);
			tcsetpgrp(1, getpid());
			write(1, "\n", 1);
			if (!strcmp(cmd->program, "pwd"))
				pwd(ishell->cwd);
			else if (!strcmp(cmd->program, "ls"))
				ls(ishell->cwd);
			else if (!strcmp(cmd->program, "cd"))
				cd(cmd);
			else if (!strcmp(cmd->program, "uname"))
				execute_program(cmd->program, cmd);
			else if (!strcmp(cmd->program, "host"))
				execute_program(cmd->program, cmd);
			else
				write(1, "command not found", 18);
			exit(0);
		}
		else
		{
			waitid(P_PID, fd, infop, WEXITED);
			write(1, "\n\21", 2);
			print_prefix(ishell->cwd);
		}
	}
}

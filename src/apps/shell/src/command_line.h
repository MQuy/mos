#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <stdbool.h>

struct command_line
{
	char *program;
	char **args;
	bool is_builtin;
};

int parse_text(char *text, int len, struct command_line *cmd);
int pwd(char *cwd);
int ls(char *path);
int cd(struct command_line *cmd);

#endif

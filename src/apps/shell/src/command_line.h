#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <stdbool.h>

struct command_line
{
	char *program;
	bool is_builtin;
};

int parse_text(char *text, struct command_line *cmd);
int pwd(char *cwd);

#endif

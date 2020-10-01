#include "command_line.h"

#include <stdlib.h>
#include <string.h>

char *clean_text(char *text, int len)
{
	int pstart = 0;
	for (; pstart < len; ++pstart)
	{
		char ch = text[pstart];
		if (ch != ' ' && ch != '\t')
			break;
	}

	int pend = len - 1;
	for (; pend >= 0; pend--)
	{
		char ch = text[pend];
		// TODO: MQ 2020-09-27 Multiple spaces like "hello   world"
		if (!isspace(ch))
			break;
	}
	char *ct = calloc(pend - pstart + 2, sizeof(char));
	memcpy(ct, text + pstart, pend - pstart + 1);
	return ct;
}

int parse_args(char *args, struct command_line *cmd)
{
	if (!args)
		return -1;

	int len = strlen(args);
	int argc = len > 0 ? 2 : 1;
	for (int i = 0; i < len; ++i)
	{
		if (isspace(args[i]))
			argc++;
	}
	char **argv = calloc(argc + 1, sizeof(char *));

	char *filename = NULL;
	char *path = NULL;
	strlsplat(cmd->program, strliof(cmd->program, "/"), &path, &filename);
	if (!filename)
		filename = cmd->program;
	argv[0] = filename;

	int pstart = 0;
	int iargv = 1;
	for (int i = 0; i < len; ++i)
	{
		if (isspace(args[i]))
		{
			argv[iargv] = calloc(i - pstart + 1, sizeof(char));
			memcpy(argv[iargv], args + pstart, i - pstart);
			pstart = i + 1;
			iargv++;
		}
	}
	argv[iargv] = calloc(len - pstart + 1, sizeof(char));
	memcpy(argv[iargv], args + pstart, len - pstart);

	cmd->args = argv;
	return 0;
}

int parse_text(char *text, int len, struct command_line *cmd)
{
	char *program = NULL;
	char *args = NULL;

	char *ct = clean_text(text, len);
	int psep = striof(ct, " ");

	if (psep != -1)
		strlsplat(ct, psep, &program, &args);
	else
		program = ct;

	cmd->program = program;
	parse_args(args, cmd);

	return 0;
}

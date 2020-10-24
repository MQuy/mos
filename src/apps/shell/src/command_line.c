#include "command_line.h"

#include <ctype.h>
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

int parse_text(char *text, int len, struct command_line *cmd)
{
	char *input = clean_text(text, len);
	int input_len = strlen(input);
	int argc = 1;
	for (int i = 0; i < input_len; ++i)
	{
		if (isspace(input[i]))
			argc++;
	}
	char **argv = calloc(argc + 1, sizeof(char *));

	for (int i = 0, iargv = 0, pstart = 0; i < input_len; ++i)
	{
		if (isspace(input[i]) || i == input_len - 1)
		{
			int iargv_len = i == input_len - 1 ? i - pstart + 1 : i - pstart;
			argv[iargv] = calloc(iargv_len + 1, sizeof(char));
			memcpy(argv[iargv], input + pstart, iargv_len);
			pstart = i + 1;
			iargv++;
		}
	}

	cmd->program = argv[0];
	cmd->args = argv;

	return 0;
}

#ifndef SHELL_H
#define SHELL_H

#define ROOT_PATH "/"
#define CHARACTERS_PER_LINE 256
#define MAX_PATH_LENGTH 256

struct shell
{
	char cwd[MAX_PATH_LENGTH];
};

#endif

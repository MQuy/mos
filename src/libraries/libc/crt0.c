#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern void _stdio_init();
extern int main(int, char**, char**);

void _start(int argc, char** argv, char** envp)
{
	environ = envp;
	_stdio_init();

	program_invocation_name = argv && argv[0] ? argv[0] : "";
	strlsplat(program_invocation_name, strliof(program_invocation_name, "/"), NULL, &program_invocation_short_name);
	if (!program_invocation_short_name)
		program_invocation_short_name = strdup(program_invocation_name);

	int code = main(argc, argv, envp);

	exit(code);
}

extern void _stdio_init();
extern int main(int, char**, char**);
extern void exit(int code);

void _start(int argc, char** argv, char** envp)
{
	_stdio_init();

	int code = main(argc, argv, envp);

	exit(code);
}

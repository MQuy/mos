#include <assert.h>
#include <stdio.h>
#include <unistd.h>

static char debug_buffer[1024] = {0};

_syscall2(dprintf, enum debug_level, const char *);
int debug_printf(enum debug_level level, const char *fmt, ...)
{
	int out;
	va_list args;
	va_start(args, fmt);

	vsprintf(debug_buffer, fmt, args);
	out = syscall_dprintf(level, debug_buffer);

	va_end(args);
	return out;
}

_syscall2(dprintln, enum debug_level, const char *);
int debug_println(enum debug_level level, const char *fmt, ...)
{
	int out;
	va_list args;
	va_start(args, fmt);

	vsprintf(debug_buffer, fmt, args);
	out = syscall_dprintln(level, debug_buffer);

	va_end(args);
	return out;
}

void __assert_debug(char *file, int line, char *func, ...)
{
	va_list args;
	va_start(args, func);
	char *message = va_arg(args, char *);
	va_end(args);

	if (message)
		debug_println(DEBUG_INFO, "%s", message);
	debug_println(DEBUG_ERROR, "%s:%d %s", file, line, func);
}

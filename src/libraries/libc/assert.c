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

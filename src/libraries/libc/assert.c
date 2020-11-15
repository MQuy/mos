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

void __dbg(enum debug_level level, bool prefix, const char *file, int line, const char *func, ...)
{
	va_list args;
	va_start(args, func);
	const char *fmt = va_arg(args, const char *);

	char msg[1024];
	if (prefix)
		sprintf(msg, "%s on %s:%d %s\n", func, file, line, fmt);
	else
		sprintf(msg, "%s\n", fmt);
	vsprintf(debug_buffer, msg, args);
	syscall_dprintf(level, debug_buffer);

	va_end(args);
}

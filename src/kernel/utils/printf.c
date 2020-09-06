#include "printf.h"

#include <kernel/devices/char/tty.h>
#include <kernel/utils/string.h>

#define SERIAL_PORT_A 0x3f8

size_t vsprintf(char *buffer, const char *fmt, va_list args)
{
	if (!fmt)
		return 0;

	char *buffer_iter = buffer;
	char *fmt_iter = fmt;
	char number_buf[32] = {0};
	for (; *fmt_iter; fmt_iter++)
	{
		if (*fmt_iter != '%')
		{
			*buffer_iter++ = *fmt_iter;
			continue;
		}

		switch (*++fmt_iter)
		{
		case 'c':
		{
			*buffer_iter++ = (char)va_arg(args, int);
			break;
		}

		case 's':
		{
			char *s = (char *)va_arg(args, char *);

			while (s && *s)
				*buffer_iter++ = *s++;
			break;
		}

		case 'd':
		case 'i':
		{
			int n = (int)va_arg(args, int);
			itoa_s(n, 10, number_buf);

			for (char *c = number_buf; *c; c++)
				*buffer_iter++ = *c;
			break;
		}

		case 'u':
		{
			unsigned int n = va_arg(args, unsigned int);
			itoa_s(n, 10, number_buf);

			for (char *c = number_buf; *c; c++)
				*buffer_iter++ = *c;
			break;
		}

		case 'l':
		{
			long long n = (long long)va_arg(args, long long);
			itoa_s(n, 10, number_buf);

			for (char *c = number_buf; *c; c++)
				*buffer_iter++ = *c;
			break;
		}

		case 'X':
		case 'x':
		{
			unsigned int n = va_arg(args, unsigned int);
			itoa_s(n, 16, number_buf);

			for (char *c = number_buf; *c; c++)
				*buffer_iter++ = *c;
			break;
		}
		default:
			*buffer_iter++ = *fmt_iter;
			break;
		}
	}

	*buffer_iter = '\0';
	return buffer_iter - buffer;
}

size_t sprintf(char *buffer, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	size_t size = vsprintf(buffer, fmt, args);

	va_end(args);

	return size;
}

// LOG
static char log_buffer[1024];
static char *tag_opening[] = {
	[DEBUG_TRACE] = "\\\\033[38;5;14m",
	[DEBUG_WARNING] = "\\\\033[38;5;11m",
	[DEBUG_ERROR] = "\\\\033[38;5;9m",
	[DEBUG_FATAL] = "\\\\033[48;5;9m",
};
static char tag_closing[] = "\\\\033[m";

void debug_write(const char *str)
{
	for (char *ch = str; *ch; ++ch)
		serial_output(SERIAL_PORT_A, *ch);
}

int debug_vsprintf(enum debug_level level, const char *fmt, va_list args)
{
	int out = vsprintf(log_buffer, fmt, args);
	if (level != DEBUG_INFO)
	{
		debug_write(tag_opening[level]);
		debug_write(log_buffer);
		debug_write(tag_closing);
	}
	else
		debug_write(log_buffer);
	return out;
}

int debug_printf(enum debug_level level, const char *fmt, ...)
{
	int out;
	va_list args;
	va_start(args, fmt);

	out = debug_vsprintf(level, fmt, args);

	va_end(args);
	return out;
}

int debug_println(enum debug_level level, const char *fmt, ...)
{
	int out;
	va_list args;
	va_start(args, fmt);

	out = debug_vsprintf(level, fmt, args);
	debug_write("\n");

	va_end(args);
	return out + 1;
}

void debug_init()
{
	serial_enable(SERIAL_PORT_A);
}

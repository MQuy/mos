#include <kernel/devices/serial.h>
#include <kernel/utils/string.h>
#include "printf.h"

size_t vasprintf(char *buffer, const char *fmt, va_list args)
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

		case 'X':
		case 'x':
		{
			unsigned int n = va_arg(args, unsigned int);
			itoa_s(n, 16, number_buf);

			memcpy(buffer_iter, "0x", 2);
			buffer_iter += 2;

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

// LOG
static char log_buffer[1024];
static char tag_opening[][24] = {
		[DEBUG_TRACE] = "\\\\033[38;5;14m",
		[DEBUG_WARNING] = "\\\\033[38;5;11m",
		[DEBUG_ERROR] = "\\\\033[38;5;9m",
		[DEBUG_FATAL] = "\\\\033[48;5;9m",
};
static char tag_closing[] = "\\\\033[m";

void debug_print_raw(const char *str)
{
	for (char *ch = str; *ch; ++ch)
		serial_write(*ch);
}

void debug_print(enum debug_level level, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vasprintf(log_buffer, fmt, args);
	if (level != DEBUG_INFO)
	{
		debug_print_raw(tag_opening[level]);
		debug_print_raw(log_buffer);
		debug_print_raw(tag_closing);
	}
	else
		debug_print_raw(log_buffer);

	va_end(args);
}

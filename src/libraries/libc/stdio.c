#include "stdio.h"

#include <string.h>
#include <unistd.h>

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

static char debug_buffer[1024] = {0};
int debug_printf(enum debug_level level, const char *fmt, ...)
{
	int out;
	va_list args;
	va_start(args, fmt);

	vsprintf(debug_buffer, fmt, args);
	out = dprintf(level, debug_buffer);

	va_end(args);
	return out;
}

int debug_println(enum debug_level level, const char *fmt, ...)
{
	int out;
	va_list args;
	va_start(args, fmt);

	vsprintf(debug_buffer, fmt, args);
	out = dprintln(level, debug_buffer);

	va_end(args);
	return out;
}

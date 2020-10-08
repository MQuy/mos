#include "stdio.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int vsprintf(char *buffer, const char *fmt, va_list args)
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

int sprintf(char *buffer, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	int size = vsprintf(buffer, fmt, args);

	va_end(args);

	return size;
}

bool valid_stream(FILE *stream)
{
	return stream->fd != -1;
}

FILE *fopen(const char *filename, const char *mode)
{
	int flags = O_RDWR;
	// don't support character b in mode
	if (mode[0] == 'r' && mode[1] != '+')
		flags = O_RDONLY;
	else if ((mode[0] == 'w' || mode[0] == 'a') && mode[1] != '+')
		flags = O_WRONLY;

	int fd = open(filename, flags, 0);
	if (fd < 0)
		return NULL;

	return fdopen(fd, mode);
}

FILE *fdopen(int fd, const char *mode)
{
	FILE *stream = calloc(1, sizeof(FILE));
	stream->fd = fd;
	stream->flags |= _IO_UNBUFFERED;

	if (mode[0] == 'r' && mode[1] != '+')
		stream->flags |= _IO_NO_WRITES;
	else if ((mode[0] == 'w' || mode[0] == 'a') && mode[1] != '+')
		stream->flags |= _IO_NO_READS;

	if (mode[0] == 'a')
	{
		stream->flags |= _IO_IS_APPENDING;
		stream->offset = lseek(fd, 0, SEEK_END);
	}

	// fill in blksize and mode
	struct stat stat;
	memset(&stat, 0, sizeof(struct stat));
	fstat(fd, &stat);

	if (S_ISREG(stat.mode))
		stream->flags |= _IO_FULLY_BUF;
	else if (S_ISCHR(stat.mode))
		stream->flags |= _IO_LINE_BUF;

	stream->blksize = stat.blksize;
	return stream;
}

int feof(FILE *stream)
{
	return stream->flags & _IO_EOF_SEEN;
}

int ferror(FILE *stream)
{
	return stream->flags & _IO_ERR_SEEN;
}

int fileno(FILE *stream)
{
	if (valid_stream(stream))
		return stream->fd;
	return -EBADF;
}

void clearerr(FILE *stream)
{
	stream->flags &= ~_IO_ERR_SEEN & ~_IO_EOF_SEEN;
}

int fgetc(FILE *stream)
{
}

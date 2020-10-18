#include <errno.h>
#include <libc-pointer-arith.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUF_LEN 4096

struct list_head lstream;

FILE *stdin, *stdout, *stderr;

void _stdio_init()
{
	INIT_LIST_HEAD(&lstream);

	stdin = fdopen(0, "r");
	stdout = fdopen(1, "w");
	stderr = fdopen(2, "w");
}

bool valid_stream(FILE *stream)
{
	return stream->fd != -1;
}

void assert_stream(FILE *stream)
{
	assert(stream->read_base <= stream->read_ptr && stream->read_ptr <= stream->read_end + 1);
	assert(stream->write_base <= stream->write_ptr && stream->write_ptr <= stream->write_end + 1);
}

FILE *fopen(const char *filename, const char *mode)
{
	int flags = O_RDWR;
	// TODO: MQ 2020-10-11 Support mode `b`
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
	stream->bkup_chr = -1;
	list_add_tail(&stream->sibling, &lstream);

	// TODO: MQ 2020-10-11 Support mode `b`
	if (mode[0] == 'r' && mode[1] != '+')
		stream->flags |= _IO_NO_WRITES;
	else if ((mode[0] == 'w' || mode[0] == 'a') && mode[1] != '+')
		stream->flags |= _IO_NO_READS;

	if (mode[0] == 'a')
	{
		stream->flags |= _IO_IS_APPENDING;
		stream->pos = lseek(fd, 0, SEEK_END);
	}

	// fill in blksize and mode
	struct stat stat = {0};
	fstat(fd, &stat);

	if (S_ISREG(stat.mode))
		stream->flags |= _IO_FULLY_BUF;
	else if (isatty(fd))
		stream->flags |= _IO_LINE_BUF;
	else
		stream->flags |= _IO_UNBUFFERED;

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
	assert_stream(stream);

	if (stream->read_ptr > stream->read_end || !stream->read_end)
	{
		if (stream->flags & _IO_EOF_SEEN)
			return EOF;

		int count = (stream->flags & _IO_FULLY_BUF || stream->flags & _IO_LINE_BUF)
						? ALIGN_UP(stream->pos + 1, max(stream->blksize, 1)) - stream->pos
						: 1;
		char *buf = calloc(count, sizeof(char));

		count = read(stream->fd, buf, count);
		// end of file
		if (!count)
		{
			stream->flags |= _IO_EOF_SEEN;
			free(buf);
			return EOF;
		}

		free(stream->read_base);
		stream->read_base = stream->read_ptr = buf;
		stream->read_end = buf + count;
	}

	stream->pos++;
	return (unsigned char)*stream->read_ptr++;
}

char *fgets(char *s, int n, FILE *stream)
{
	int i;
	for (i = 0; i < n; ++i)
	{
		int ch = fgetc(stream);

		if (ch == '\n')
			break;
		if (ch == EOF)
			return NULL;

		s[i] = ch;
	}

	s[i] = 0;
	return s;
}

size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream)
{
	size_t i;
	for (i = 0; i < nitems; ++i)
	{
		char *buf = calloc(size, sizeof(char));
		if (!fgets(buf, size, stream))
		{
			free(buf);
			break;
		}

		memcpy((char *)ptr + i * size, buf, size);
		free(buf);
	}
	return i;
}

long int ftell(FILE *stream)
{
	return stream->pos;
}

off_t ftello(FILE *stream)
{
	return stream->pos;
}

int getchar()
{
	return getc(stdin);
}

int ungetc(int c, FILE *stream)
{
	assert_stream(stream);

	if (c == EOF || stream->bkup_chr != -1)
		return EOF;

	if (stream->read_ptr == stream->read_base)
	{
		int size = stream->read_end - stream->read_base + 2;
		char *buf = calloc(size, sizeof(char));

		memcpy(buf + 1, stream->read_base, size - 1);
		free(stream->read_base);

		stream->read_base = buf;
		stream->read_ptr = buf + 1;
		stream->read_end = buf + size;
	}
	stream->pos--;
	stream->bkup_chr = *stream->read_ptr--;
	*stream->read_ptr = (unsigned char)c;
	stream->flags &= ~_IO_EOF_SEEN;
	return (unsigned char)c;
}

void rewind(FILE *stream)
{
	fseek(stream, 0, SEEK_SET);
	stream->flags &= ~_IO_ERR_SEEN;
}

int fseek(FILE *stream, long int off, int whence)
{
	assert_stream(stream);

	fflush(stream);

	struct stat stat = {0};
	fstat(stream->fd, &stat);

	loff_t offset = off;
	if (whence == SEEK_CUR)
		offset = stream->pos + off;
	else if (whence == SEEK_END)
		offset = stat.size + off;
	stream->pos = offset;
	lseek(stream->fd, offset, SEEK_SET);

	free(stream->read_base);
	stream->read_base = stream->read_end = stream->read_ptr = NULL;
	stream->bkup_chr = -1;

	stream->flags &= ~_IO_EOF_SEEN;
	return 0;
}

int fseeko(FILE *stream, off_t offset, int whence)
{
	return fseek(stream, offset, whence);
}

#define MIN_WRITE_BUF_LEN 32

int fnput(const char *s, int size, FILE *stream)
{
	assert_stream(stream);

	int remaining_len = stream->write_end - stream->write_ptr;
	int current_len = stream->write_end - stream->write_base;

	if (remaining_len < size)
	{
		int new_len = max(max(size, current_len * 2), MIN_WRITE_BUF_LEN);
		char *buf = calloc(new_len, sizeof(char));

		memcpy(buf, stream->write_base, current_len);
		free(stream->write_base);

		stream->write_base = buf;
		stream->write_ptr = buf + current_len - remaining_len;
		stream->write_end = buf + new_len;
	}
	memcpy(stream->write_ptr, s, size);
	stream->write_ptr += size;
	stream->pos += size;
	return size;
}

int fputc(int c, FILE *stream)
{
	char ch = (unsigned char)c;
	fnput(&ch, 1, stream);
	return ch;
}

int putchar(int c)
{
	return putc(c, stdout);
}

int fputs(const char *s, FILE *stream)
{
	assert_stream(stream);

	int slen = strlen(s);
	return fnput(s, slen, stream);
}

int puts(const char *s)
{
	fputs(s, stdout);
	return fputc('\n', stdout);
}

size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream)
{
	int i;
	for (i = 0; i < nitems; ++i)
	{
		if (fnput((char *)ptr + i * size, size, stream) == EOF)
			break;
	}

	return i * size;
}

int fflush(FILE *stream)
{
	if (!stream)
	{
		FILE *iter;
		list_for_each_entry(iter, &lstream, sibling)
		{
			if (!(iter->flags & _IO_NO_WRITES))
				fflush(iter);
		}
		return 0;
	}

	assert_stream(stream);
	if (!valid_stream(stream))
		return -EBADF;

	int unwritten_len = stream->write_ptr - stream->write_base;

	if (!unwritten_len)
		return 0;

	write(stream->fd, stream->write_base, unwritten_len);
	free(stream->write_base);

	stream->write_base = stream->write_ptr = stream->write_end = NULL;
	return 0;
}

int fclose(FILE *stream)
{
	assert_stream(stream);
	if (!valid_stream(stream))
		return -EBADF;

	fflush(stream);
	free(stream->read_base);
	stream->read_base = stream->read_ptr = stream->read_end = NULL;

	close(stream->fd);
	stream->fd = -1;

	return 0;
}

int fgetpos(FILE *stream, fpos_t *pos)
{
	*pos = stream->pos;
	return 0;
}

int fsetpos(FILE *stream, const fpos_t *pos)
{
	fflush(stream);

	stream->pos = *pos;
	stream->flags &= ~_IO_EOF_SEEN;

	if (stream->bkup_chr == -1)
		*stream->write_ptr++ = stream->bkup_chr;

	return 0;
}

int vfprintf(FILE *stream, const char *fmt, va_list args)
{
	char text[MAX_BUF_LEN] = {0};
	vsprintf(text, fmt, args);

	return fputs(text, stream);
}

int vprintf(const char *fmt, va_list args)
{
	return vfprintf(stdout, fmt, args);
}

int fprintf(FILE *stream, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	int ret = vfprintf(stream, fmt, args);

	va_end(args);
	return ret;
}

int printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	int ret = vprintf(fmt, args);

	va_end(args);
	return ret;
}

int vfscanf(FILE *stream, const char *fmt, va_list args)
{
	char text[MAX_BUF_LEN] = {0};
	fgets(text, MAX_BUF_LEN, stream);

	return vsscanf(text, fmt, args);
}

int fscanf(FILE *stream, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	int ret = vfscanf(stream, fmt, args);

	va_end(args);
	return ret;
}

int scanf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	int ret = vfscanf(stdin, fmt, args);

	va_end(args);
	return ret;
}

FILE *tmpfile()
{
	static int tmpfile_num = 1;

	char tmp[100];
	sprintf(tmp, "/tmp/tmp%d.%d", getpid(), tmpfile_num++);

	return fopen(tmp, "w+");
}

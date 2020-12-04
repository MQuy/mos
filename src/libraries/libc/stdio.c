#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <libc-pointer-arith.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
	assert(stream->_IO_read_base <= stream->_IO_read_ptr && stream->_IO_read_ptr <= stream->_IO_read_end + 1);
	assert(stream->_IO_write_base <= stream->_IO_write_ptr && stream->_IO_write_ptr <= stream->_IO_write_end + 1);
}

FILE *fopen(const char *filename, const char *mode)
{
	int flags = O_RDWR;
	// TODO: MQ 2020-10-11 Support mode `b`
	if (mode[0] == 'r' && mode[1] != '+')
		flags = O_RDONLY;
	else if ((mode[0] == 'w' || mode[0] == 'a') && mode[1] != '+')
		flags = O_WRONLY;

	if ((mode[0] == 'w' && mode[1] == '+') || mode[0] == 'a')
		flags |= O_CREAT;

	int fd = open(filename, flags | O_CREAT);
	if (fd < 0)
		return NULL;

	return fdopen(fd, mode);
}

void fchange_mode(FILE *stream, const char *mode)
{
	// TODO: MQ 2020-10-11 Support mode `b`
	if (mode[0] == 'r' && mode[1] != '+')
		stream->_flags |= _IO_NO_WRITES;
	else if ((mode[0] == 'w' || mode[0] == 'a') && mode[1] != '+')
		stream->_flags |= _IO_NO_READS;

	if (mode[0] == 'a')
	{
		stream->_flags |= _IO_IS_APPENDING;
		stream->_offset = lseek(stream->fd, 0, SEEK_END);
	}
}

FILE *fdopen(int fd, const char *mode)
{
	FILE *stream = calloc(1, sizeof(FILE));
	stream->fd = fd;
	stream->bkup_chr = -1;
	list_add_tail(&stream->sibling, &lstream);

	fchange_mode(stream, mode);

	// fill in blksize and mode
	struct stat stat = {0};
	fstat(fd, &stat);

	if (S_ISREG(stat.st_mode))
		stream->_flags |= _IO_FULLY_BUF;
	else if (isatty(fd))
		stream->_flags |= _IO_LINE_BUF;
	else
		stream->_flags |= _IO_UNBUFFERED;

	stream->blksize = stat.st_blksize;
	return stream;
}

FILE *freopen(const char *pathname, const char *mode,
			  FILE *stream)
{
	if (valid_stream(stream))
		return errno = -EBADF, NULL;

	fflush(stream);
	if (!pathname)
		fclose(stream);
	clearerr(stream);

	FILE *newstream = stream;
	if (pathname)
	{
		newstream = fopen(pathname, mode);
		fclose(stream);
	}
	else
		fchange_mode(stream, mode);

	return newstream;
}

int feof(FILE *stream)
{
	return stream->_flags & _IO_EOF_SEEN;
}

int ferror(FILE *stream)
{
	return stream->_flags & _IO_ERR_SEEN;
}

int fileno(FILE *stream)
{
	if (valid_stream(stream))
		return stream->fd;
	return -EBADF;
}

void clearerr(FILE *stream)
{
	stream->_flags &= ~(_IO_ERR_SEEN || _IO_EOF_SEEN);
}

static size_t fnget(void *ptr, size_t size, FILE *stream)
{
	assert_stream(stream);

	if (stream->_IO_read_ptr + size > stream->_IO_read_end || !stream->_IO_read_end)
	{
		if (stream->_flags & _IO_EOF_SEEN)
			return EOF;

		int count = (stream->_flags & _IO_FULLY_BUF || stream->_flags & _IO_LINE_BUF)
						? ALIGN_UP(stream->_offset + size, max(stream->blksize, 1)) - stream->_offset
						: size;
		char *buf = calloc(count, sizeof(char));

		count = read(stream->fd, buf, count);
		// end of file
		if (!count)
		{
			stream->_flags |= _IO_EOF_SEEN;
			free(buf);
			return EOF;
		}

		free(stream->_IO_read_base);
		stream->_IO_read_base = stream->_IO_read_ptr = buf;
		stream->_IO_read_end = buf + count;
	}

	memcpy(ptr, stream->_IO_read_ptr, size);
	stream->_offset += size;
	stream->_IO_read_ptr += size;
	return size;
}

int fgetc(FILE *stream)
{
	char ch;
	int ret = fnget(&ch, 1, stream);
	return ret == EOF ? ret : (unsigned char)ch;
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
		int count = fnget((char *)ptr + i * size, size, stream);
		if (count == EOF)
			return i;
	}
	return i;
}

long int ftell(FILE *stream)
{
	return stream->_offset;
}

off_t ftello(FILE *stream)
{
	return stream->_offset;
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

	if (stream->_IO_read_ptr == stream->_IO_read_base)
	{
		int size = stream->_IO_read_end - stream->_IO_read_base + 2;
		char *buf = calloc(size, sizeof(char));

		memcpy(buf + 1, stream->_IO_read_base, size - 1);
		free(stream->_IO_read_base);

		stream->_IO_read_base = buf;
		stream->_IO_read_ptr = buf + 1;
		stream->_IO_read_end = buf + size;
	}
	stream->_offset--;
	stream->bkup_chr = *stream->_IO_read_ptr--;
	*stream->_IO_read_ptr = (unsigned char)c;
	stream->_flags &= ~_IO_EOF_SEEN;
	return (unsigned char)c;
}

void rewind(FILE *stream)
{
	fseek(stream, 0, SEEK_SET);
	stream->_flags &= ~_IO_ERR_SEEN;
}

int fseek(FILE *stream, long int off, int whence)
{
	assert_stream(stream);

	fflush(stream);

	struct stat stat = {0};
	fstat(stream->fd, &stat);

	loff_t offset = off;
	if (whence == SEEK_CUR)
		offset = stream->_offset + off;
	else if (whence == SEEK_END)
		offset = stat.st_size + off;
	stream->_offset = offset;
	lseek(stream->fd, offset, SEEK_SET);

	free(stream->_IO_read_base);
	stream->_IO_read_base = stream->_IO_read_end = stream->_IO_read_ptr = NULL;
	stream->bkup_chr = -1;

	stream->_flags &= ~_IO_EOF_SEEN;
	return 0;
}

int fseeko(FILE *stream, off_t offset, int whence)
{
	return fseek(stream, offset, whence);
}

#define MIN_WRITE_BUF_LEN 32

static int fnput(const char *s, int size, FILE *stream)
{
	assert_stream(stream);

	int remaining_len = stream->_IO_write_end - stream->_IO_write_ptr;
	int current_len = stream->_IO_write_end - stream->_IO_write_base;

	if (remaining_len < size)
	{
		int new_len = max(max(size, current_len * 2), MIN_WRITE_BUF_LEN);
		char *buf = calloc(new_len, sizeof(char));

		memcpy(buf, stream->_IO_write_base, current_len);
		free(stream->_IO_write_base);

		stream->_IO_write_base = buf;
		stream->_IO_write_ptr = buf + current_len - remaining_len;
		stream->_IO_write_end = buf + new_len;
	}
	memcpy(stream->_IO_write_ptr, s, size);
	stream->_IO_write_ptr += size;
	stream->_offset += size;
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
			if (!(iter->_flags & _IO_NO_WRITES))
				fflush(iter);
		}
		return 0;
	}

	assert_stream(stream);
	if (!valid_stream(stream))
		return -EBADF;

	int unwritten_len = stream->_IO_write_ptr - stream->_IO_write_base;

	if (!unwritten_len)
		return 0;

	write(stream->fd, stream->_IO_write_base, unwritten_len);
	free(stream->_IO_write_base);

	stream->_IO_write_base = stream->_IO_write_ptr = stream->_IO_write_end = NULL;
	return 0;
}

int fclose(FILE *stream)
{
	assert_stream(stream);
	if (!valid_stream(stream))
		return -EBADF;

	fflush(stream);
	free(stream->_IO_read_base);
	stream->_IO_read_base = stream->_IO_read_ptr = stream->_IO_read_end = NULL;

	close(stream->fd);
	stream->fd = -1;

	return 0;
}

int fgetpos(FILE *stream, fpos_t *pos)
{
	*pos = stream->_offset;
	return 0;
}

int fsetpos(FILE *stream, const fpos_t *pos)
{
	fflush(stream);

	stream->_offset = *pos;
	stream->_flags &= ~_IO_EOF_SEEN;

	if (stream->bkup_chr == -1)
		*stream->_IO_write_ptr++ = stream->bkup_chr;

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

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
	assert_stream(stream);

	if (mode == _IOFBF)
	{
		stream->_flags &= ~(_IO_LINE_BUF | _IO_UNBUFFERED);
		stream->_flags |= _IO_FULLY_BUF;
	}
	else if (mode == _IOLBF)
	{
		stream->_flags &= ~(_IO_FULLY_BUF | _IO_UNBUFFERED);
		stream->_flags |= _IO_LINE_BUF;
	}
	else
	{
		stream->_flags &= ~(_IO_FULLY_BUF | _IO_LINE_BUF);
		stream->_flags |= _IO_UNBUFFERED;
	}

	// TODO: MQ 2020-10-26 Implement customized stream buffer
	return 0;
}

void setbuf(FILE *stream, char *buf)
{
	setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}

void flockfile(FILE *fp)
{
	assert_not_implemented();
}

void funlockfile(FILE *fp)
{
	assert_not_implemented();
}

_syscall2(rename, const char *, const char *);
int rename(const char *oldpath, const char *newpath)
{
	SYSCALL_RETURN(syscall_rename(oldpath, newpath));
}

_syscall4(renameat, int, const char *, int, const char *);
int renameat(int olddirfd, const char *oldpath,
			 int newdirfd, const char *newpath)
{
	SYSCALL_RETURN(syscall_renameat(olddirfd, oldpath, newdirfd, newpath));
}

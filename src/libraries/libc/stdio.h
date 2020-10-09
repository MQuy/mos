#ifndef LIBC_STDIO_H
#define LIBC_STDIO_H

#include <shared/fcntl.h>
#include <shared/vsprintf.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>

#define EOF (-1)

#define _IO_USER_BUF 0x0001 /* Don't deallocate buffer on close. */
#define _IO_UNBUFFERED 0x0002
#define _IO_NO_READS 0x0004	 /* Reading not allowed.  */
#define _IO_NO_WRITES 0x0008 /* Writing not allowed.  */
#define _IO_EOF_SEEN 0x0010
#define _IO_ERR_SEEN 0x0020
#define _IO_DELETE_DONT_CLOSE 0x0040 /* Don't call close(_fileno) on close.  */
#define _IO_LINKED 0x0080			 /* In the list of all open files.  */
#define _IO_IN_BACKUP 0x0100
#define _IO_LINE_BUF 0x0200
#define _IO_TIED_PUT_GET 0x0400 /* Put and get pointer move in unison.  */
#define _IO_CURRENTLY_PUTTING 0x0800
#define _IO_IS_APPENDING 0x1000
#define _IO_IS_FILEBUF 0x2000
#define _IO_FULLY_BUF 0x4000

struct __FILE
{
	int fd;
	int flags;
	int pos;
	char *read_ptr, *read_base, *read_end;
	char *write_ptr, *write_base, *write_end;
	int bkup_chr;  // for ungetc
	int blksize;
};
typedef struct __FILE FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

FILE *fopen(const char *filename, const char *mode);
FILE *fdopen(int fd, const char *mode);
int feof(FILE *stream);
int ferror(FILE *stream);
int fileno(FILE *stream);
void clearerr(FILE *stream);
int fgetc(FILE *stream);
char *fgets(char *s, int n, FILE *stream);
size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream);
long int ftell(FILE *stream);
off_t ftello(FILE *stream);
void rewind(FILE *stream);
#define getc(stream) fgetc(stream)
int getchar();
int ungetc(int c, FILE *stream);
int fseek(FILE *stream, long int offset, int whence);
int fseeko(FILE *stream, off_t offset, int whence);

// int fflush(FILE *);
// int fgetpos(FILE *, fpos_t *);
// int fprintf(FILE *, const char *, ...);
// int fputc(int, FILE *);
// int fputs(const char *, FILE *);
// FILE *freopen(const char *, const char *, FILE *);
// int fscanf(FILE *, const char *, ...);
// int fsetpos(FILE *, const fpos_t *);
// size_t fwrite(const void *, size_t, size_t, FILE *);
// int getopt(int, char *const[], const char);
// int getw(FILE *);
// int pclose(FILE *);
// void perror(const char *);
// FILE *popen(const char *, const char *);
// int printf(const char *, ...);
// int putc(int, FILE *);
// int putchar(int);
// int puts(const char *);
// int putw(int, FILE *);
// int remove(const char *);
// int rename(const char *, const char *);
// void setbuf(FILE *, char *);
// int setvbuf(FILE *, char *, int, size_t);
// char *tempnam(const char *, const char *);
// FILE *tmpfile(void);
// char *tmpnam(char *);
// int vfprintf(FILE *, const char *, va_list);
// int vprintf(const char *, va_list);

#endif

#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H 1

#include <list.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>

#define BUFSIZ 512
#define _IOFBF 0 /* Fully buffered. */
#define _IOLBF 1 /* Line buffered. */
#define _IONBF 2 /* No buffering. */

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

#ifndef SEEK_SET
#define SEEK_SET 0 /* Seek from beginning of file.  */
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1 /* Seek from current position.  */
#endif

#ifndef SEEK_END
#define SEEK_END 2 /* Seek from end of file.  */
#endif

#ifndef __FILE_defined
#define __FILE_defined
#include <FILE.h>
typedef struct __FILE FILE;
#endif

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
int fputc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);
int puts(const char *);
int fflush(FILE *stream);
int fclose(FILE *stream);
int fgetpos(FILE *stream, fpos_t *pos);
int fsetpos(FILE *stream, const fpos_t *pos);
#define putc(c, stream) fputc(c, stream)
int putchar(int c);
int fprintf(FILE *stream, const char *format, ...);
int printf(const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list ap);
int vprintf(const char *format, va_list ap);
size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
int fscanf(FILE *stream, const char *format, ...);
int scanf(const char *format, ...);
int setvbuf(FILE *stream, char *buf, int mode, size_t size);
void setbuf(FILE *stream, char *buf);
FILE *tmpfile();

int rename(const char *oldpath, const char *newpath);
int renameat(int olddirfd, const char *oldpath,
			 int newdirfd, const char *newpath);
int renameat2(int olddirfd, const char *oldpath,
			  int newdirfd, const char *newpath, unsigned int flags);

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int vsscanf(const char *buf, const char *fmt, va_list args);
int sscanf(const char *buf, const char *fmt, ...);
int asprintf(char **strp, const char *fmt, ...);
int vasprintf(char **strp, const char *fmt, va_list ap);
int vcbprintf(void *ctx,
			  size_t (*callback)(void *, const char *, size_t),
			  const char *format,
			  va_list parameters);
int vcbscanf(void *fp,
			 int (*fgetc)(void *),
			 int (*ungetc)(int, void *),
			 const char *restrict format,
			 va_list ap);

void flockfile(FILE *fp);
void funlockfile(FILE *fp);

#endif

#ifndef LIBC_SYS_REENT_H
#define LIBC_SYS_REENT_H

struct _reent;

struct __sbuf
{
	unsigned char *_base;
	int _size;
};

struct __FILE
{
	unsigned char *_p; /* current position in (some) buffer */
	int _r;			   /* read space left for getc() */
	int _w;			   /* write space left for putc() */
	short _flags;	   /* flags, below; this FILE is free if 0 */
	short _file;	   /* fileno, if Unix descriptor, else -1 */
	struct __sbuf _bf; /* the buffer (at least 1 byte, if !NULL) */
	int _lbfsize;	   /* 0 or -_bf._size, for inline putc */
	struct _reent *_data;
};

typedef struct __FILE FILE;

#endif

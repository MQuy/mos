#ifndef _LIBC_SETJMP_H
#define _LIBC_SETJMP_H 1

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>

struct __jmp_buf
{
	uint32_t regs[6];
	bool did_save_signal_mask;
	sigset_t saved_signal_mask;
};

typedef struct __jmp_buf jmp_buf[1];
typedef struct __jmp_buf sigjmp_buf[1];

int setjmp(jmp_buf);
void longjmp(jmp_buf, int);
int sigsetjmp(sigjmp_buf, int);
void siglongjmp(sigjmp_buf, int);

#endif

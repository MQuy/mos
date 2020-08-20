#ifndef IPC_SIGNAL_H
#define IPC_SIGNAL_H

#include <include/ctype.h>
#include <kernel/cpu/idt.h>
#include <stdint.h>

#define NSIG 32

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGIOT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGURG 23
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGIO 29
#define SIGPOLL SIGIO
/*
#define SIGLOST		29
*/
#define SIGPWR 30
#define SIGSYS 31
#define SIGUNUSED 31

/* These should not be considered constants from userland.  */
#define SIGRTMIN 32
#define SIGRTMAX NSIG

#define SA_RESTART 0x10000000u
#define SA_NODEFER 0x40000000u
#define SA_RESETHAND 0x80000000u

#define SIG_BLOCK 0	  /* for blocking signals */
#define SIG_UNBLOCK 1 /* for unblocking signals */
#define SIG_SETMASK 2 /* for setting the signal mask */

typedef void (*__sighandler_t)(int);

#define SIG_DFL ((__sighandler_t)0)	 /* default signal handling */
#define SIG_IGN ((__sighandler_t)1)	 /* ignore signal */
#define SIG_ERR ((__sighandler_t)-1) /* error return from signal */

struct sigaction
{
	__sighandler_t sa_handler;
	uint32_t sa_flags;
	sigset_t sa_mask;
};

static inline int valid_signal(unsigned long sig)
{
	return sig <= NSIG ? 1 : 0;
}

int do_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int do_sigaction(int signum, const struct sigaction *action, struct sigaction *old_action);
int do_kill(pid_t pid, int32_t signum);
void signal_handle();
void sigreturn(struct interrupt_registers *regs);

#endif

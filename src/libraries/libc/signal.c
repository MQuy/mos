#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

_syscall2(signal, int, sighandler_t);
int signal(int signum, sighandler_t handler)
{
	SYSCALL_RETURN_ORIGINAL(syscall_signal(signum, handler));
}

_syscall3(sigaction, int, const struct sigaction *, struct sigaction *);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
	SYSCALL_RETURN(syscall_sigaction(signum, act, oldact));
}

_syscall3(sigprocmask, int, const sigset_t *, sigset_t *);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	SYSCALL_RETURN(syscall_sigprocmask(how, set, oldset));
}

_syscall2(kill, pid_t, int);
int kill(pid_t pid, int sig)
{
	SYSCALL_RETURN(syscall_kill(pid, sig));
}

int raise(int32_t sig)
{
	return kill(getpid(), sig);
}

void sigfillset(sigset_t *set)
{
	*set = -1;
}

void sigemptyset(sigset_t *set)
{
	memset(set, 0, sizeof(sigset_t));
}

_syscall1(sigsuspend, const sigset_t *);
int sigsuspend(const sigset_t *mask)
{
	return errno = -syscall_sigsuspend(mask), -1;
}

int sigismember(sigset_t *set, int sig)
{
	return 1 & (*set >> (sig - 1));
}

int sigaddset(sigset_t *set, int sig)
{
	*set |= 1 << (sig - 1);
	return 0;
}

int sigdelset(sigset_t *set, int sig)
{
	*set &= ~(1 << (sig - 1));
	return 0;
}

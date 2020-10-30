#include <signal.h>
#include <string.h>
#include <unistd.h>

_syscall2(signal, int, sighandler_t);
int signal(int signum, sighandler_t handler)
{
	return syscall_signal(signum, handler);
}

_syscall3(sigaction, int, const struct sigaction *, struct sigaction *);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
	return syscall_sigaction(signum, act, oldact);
}

_syscall3(sigprocmask, int, const sigset_t *, sigset_t *);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	return syscall_sigprocmask(how, set, oldset);
}

_syscall2(kill, pid_t, int);
int kill(pid_t pid, int sig)
{
	return syscall_kill(pid, sig);
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
	return syscall_sigsuspend(mask);
}

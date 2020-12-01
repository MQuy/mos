#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

int wait(int *wstatus)
{
	return waitpid(-1, wstatus, 0);
}

_syscall3(waitpid, pid_t, int *, int);
pid_t waitpid(pid_t pid, int *wstatus, int options)
{
	SYSCALL_RETURN_ORIGINAL(syscall_waitpid(pid, wstatus, options));
}

_syscall4(waitid, idtype_t, id_t, struct infop *, int);
int waitid(idtype_t idtype, id_t id, struct infop *infop, int options)
{
	SYSCALL_RETURN(syscall_waitid(idtype, id, infop, options));
}

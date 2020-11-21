#ifndef _LIBC_WAIT_H
#define _LIBC_WAIT_H 1

#include <stdint.h>
#include <sys/types.h>

#define WNOHANG 0x00000001
#define WUNTRACED 0x00000002
#define WSTOPPED WUNTRACED
#define WEXITED 0x00000004
#define WCONTINUED 0x00000008
#define WNOWAIT 0x01000000 /* Don't reap, just poll status.  */

#define CLD_EXITED 1	/* child has exited */
#define CLD_KILLED 2	/* child was killed */
#define CLD_DUMPED 3	/* child terminated abnormally */
#define CLD_TRAPPED 4	/* traced child has trapped */
#define CLD_STOPPED 5	/* child has stopped */
#define CLD_CONTINUED 6 /* stopped child has continued */

#define P_ALL 0
#define P_PID 1
#define P_PGID 2

#define WSEXITED 0x0001
#define WSSIGNALED 0x0002
#define WSCOREDUMP 0x0004
#define WSSTOPPED 0x0008
#define WSCONTINUED 0x0010

#define WIFEXITED(wstatus) (wstatus & WSEXITED)
#define WEXITSTATUS(wstatus) (WIFEXITED(wstatus) && (wstatus >> 8) & 0xff)
#define WIFSIGNALED(wstatus) (wstatus & WSSIGNALED)
#define WTERMSIG(wstatus) (WIFSIGNALED(wstatus) && (wstatus >> 8) & 0xff)
#define WCOREDUMP(wstatus) (WIFSIGNALED(wstatus) && wstatus && WSCOREDUMP)
#define WIFSTOPPED(wstatus) (wstatus & WSSTOPPED)
#define WSTOPSIG(wstatus) (WIFSTOPPED(wstatus) && (wstatus >> 8) & 0xff)
#define WIFCONTINUED(wstatus) (wstatus & WSCONTINUED)

struct infop
{
	pid_t si_pid;
	int32_t si_signo;
	int32_t si_status;
	int32_t si_code;
};

pid_t wait(int *wstatus);
pid_t waitpid(pid_t pid, int *wstatus, int options);
int waitid(idtype_t idtype, id_t id, struct infop *infop, int options);

#endif

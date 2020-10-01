#ifndef LIBC_WAIT_H
#define LIBC_WAIT_H

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

struct infop
{
	pid_t si_pid;
	int32_t si_signo;
	int32_t si_status;
	int32_t si_code;
};

#endif

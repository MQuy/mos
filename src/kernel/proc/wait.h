#ifndef PROC_WAIT_H
#define PROC_WAIT_H

#include <include/ctype.h>
#include <include/list.h>
#include <stdint.h>

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

struct thread;

typedef void (*wait_queue_func)(struct thread *);

struct wait_queue_head
{
	struct list_head list;
};

struct wait_queue_entry
{
	struct thread *thread;
	wait_queue_func func;
	struct list_head sibling;
};

extern volatile struct thread *current_thread;
extern void schedule();

#define DEFINE_WAIT(name)            \
	struct wait_queue_entry name = { \
		.thread = current_thread,    \
		.func = poll_wakeup,         \
	}

// NOTE: MQ 2020-08-17 continue if receiving a signal
#define wait_until(cond) ({                            \
	for (; !(cond);)                                   \
	{                                                  \
		update_thread(current_thread, THREAD_WAITING); \
		schedule();                                    \
	}                                                  \
})

#define wait_until_with_prework(cond, prework) ({      \
	for (; !(cond);)                                   \
	{                                                  \
		prework;                                       \
		update_thread(current_thread, THREAD_WAITING); \
		schedule();                                    \
	}                                                  \
})

#define wait_until_with_setup(cond, prework, afterwork) ({ \
	for (; !(cond);)                                       \
	{                                                      \
		prework;                                           \
		update_thread(current_thread, THREAD_WAITING);     \
		schedule();                                        \
		afterwork;                                         \
	}                                                      \
})

#define wait_event(wh, cond) ({                  \
	DEFINE_WAIT(__wait);                         \
	list_add_tail(&__wait.sibling, &(wh)->list); \
	wait_until(cond);                            \
	list_del(&__wait.sibling);                   \
})

#endif

#include "signal.h"

#include <include/bitops.h>
#include <include/errno.h>
#include <kernel/proc/task.h>
#include <kernel/utils/hashmap.h>
#include <kernel/utils/string.h>

void sigjump_usermode(struct interrupt_registers *);

struct signal_frame
{
	void (*sigreturn)(struct interrupt_registers *);
	int32_t signum;
	int32_t blocked;
	struct interrupt_registers uregs;
};

#define M(sig) (1UL << ((sig)-1))
#define T(sig, mask) (M(sig) & (mask))

#define SIG_KERNEL_ONLY_MASK (M(SIGKILL) | M(SIGSTOP))

#define SIG_KERNEL_STOP_MASK (M(SIGSTOP) | M(SIGTSTP) | M(SIGTTIN) | M(SIGTTOU))

#define SIG_KERNEL_COREDUMP_MASK (                     \
	M(SIGQUIT) | M(SIGILL) | M(SIGTRAP) | M(SIGABRT) | \
	M(SIGFPE) | M(SIGSEGV) | M(SIGBUS) | M(SIGSYS) |   \
	M(SIGXCPU) | M(SIGXFSZ) | M_SIGEMT)

#define SIG_KERNEL_IGNORE_MASK ( \
	M(SIGCONT) | M(SIGCHLD) | M(SIGWINCH) | M(SIGURG))

#define sig_kernel_only(sig) \
	(((sig) < SIGRTMIN) && T(sig, SIG_KERNEL_ONLY_MASK))

#define sig_user_defined(p, signr)                      \
	(((p)->sighand[(signr)-1].sa_handler != SIG_DFL) && \
	 ((p)->sighand[(signr)-1].sa_handler != SIG_IGN))

int next_signal(sigset_t pending, sigset_t blocked)
{
	sigset_t mask = pending & ~blocked;
	if (mask)
		return ffz(~mask) + 1;
	return -1;
}

int do_sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	if (oldset)
		*oldset = current_thread->blocked;

	if (set)
	{
		if (how == SIG_BLOCK)
			current_thread->blocked |= *set;
		else if (how == SIG_UNBLOCK)
			current_thread->blocked &= ~*set;
		else if (how == SIG_SETMASK)
			current_thread->blocked = *set;
		else
			return -EINVAL;

		current_thread->blocked &= ~SIG_KERNEL_ONLY_MASK;
	}

	return 0;
}

int do_sigaction(int signum, const struct sigaction *action, struct sigaction *old_action)
{
	if (!valid_signal(signum) || signum < 1 || sig_kernel_only(signum))
		return -EINVAL;

	if (old_action)
		memcpy(old_action, &current_process->sighand[signum - 1], sizeof(struct sigaction));

	if (action)
		memcpy(&current_process->sighand[signum - 1], action, sizeof(struct sigaction));

	return 0;
}

int do_kill(pid_t pid, int32_t signum)
{
	if (!valid_signal(signum) || signum < 0)
		return -EINVAL;

	// TODO: MQ 2020-08-20 should do existence and permission check of process or group process id
	if (!signum)
		return 0;

	if (pid > 0)
	{
		if (current_process->pid == pid)
		{
			if (T(current_thread->blocked, signum))
				return 0;

			current_thread->pending |= M(signum);
			if (!current_thread->signaling)
				signal_handle();
		}
		else
		{
			struct process *p = find_process_by_pid(pid);
			struct thread *iter, *t = NULL;
			list_for_each_entry(iter, &p->threads, sibling)
			{
				if (!T(iter->blocked, signum) && !T(iter->pending, signum))
				{
					t = iter;
					break;
				}
			}

			if (t)
			{
				t->pending |= M(signum);
				update_thread(t, THREAD_READY);
			}
		}
	}
	else if (pid == 0)
	{
		struct process *p;
		struct hashmap_iter *iter;
		for_each_process(p, iter)
		{
			if (p->gid == current_process->gid)
				do_kill(p->gid, signum);
		}
	}
	else if (pid == -1)
	{
		struct process *p;
		struct hashmap_iter *iter;
		for_each_process(p, iter)
		{
			// TODO: MQ 2020-08-20 Make sure calling process has permission to send signals
			if (p->pid > 1)
				do_kill(p->gid, signum);
		}
	}
	else
	{
		struct process *p;
		struct hashmap_iter *iter;
		for_each_process(p, iter)
		{
			if (p->gid == -pid)
				do_kill(p->gid, signum);
		}
	}

	return 0;
}

void signal_handle()
{
	struct interrupt_registers *kregs = (struct interrupt_registers *)(current_thread->kernel_stack - sizeof(struct interrupt_registers));
	bool is_syscall_return;

	if (!current_thread->pending)
	{
		if (current_thread->signaling)
		{
			memcpy(kregs, &current_thread->uregs, sizeof(struct interrupt_registers));
			current_thread->signaling = false;
		}
		return;
	}

	if (!current_thread->signaling)
	{
		if (kregs->int_no == 0x7F)
		{
			is_syscall_return = true;
			kregs->eax = -EINTR;
		}
		memcpy(&current_thread->uregs, kregs, sizeof(struct interrupt_registers));
	}

	current_thread->signaling = true;

	int signum = next_signal(current_thread->pending, current_thread->blocked);
	set_bit(signum - 1, &current_thread->pending);

	struct sigaction *sigaction = &current_process->sighand[signum - 1];
	if (!sig_user_defined(current_process, signum))
	{
		sigset_t sigblocked = current_thread->blocked;
		current_thread->blocked |= M(signum) | sigaction->sa_mask;
		sigaction->sa_handler(signum);
		current_thread->blocked = sigblocked;
		signal_handle();
	}
	else
	{
		kregs->useresp -= sizeof(struct signal_frame);
		struct signal_frame *frame = (struct signal_frame *)kregs->useresp;
		frame->sigreturn = sigreturn;
		frame->signum = signum;
		frame->blocked = current_thread->blocked;
		frame->uregs = *kregs;

		kregs->eip = (uint32_t)sigaction->sa_handler;
		current_thread->blocked |= M(signum) | sigaction->sa_mask;
		if (is_syscall_return)
			sigjump_usermode(&current_thread->uregs);
	}
}

void sigreturn(struct interrupt_registers *regs)
{
	struct signal_frame *frame = (struct signal_frame *)regs->useresp;
	current_thread->uregs = frame->uregs;
	current_thread->blocked = frame->blocked;

	regs->useresp += sizeof(struct signal_frame);
	signal_handle();
}

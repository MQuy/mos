#include <devices/char/tty.h>
#include <include/atomic.h>
#include <ipc/signal.h>
#include <utils/printf.h>

#include "task.h"

static void exit_mm(struct process *proc)
{
	struct vm_area_struct *iter, *next;
	list_for_each_entry_safe(iter, next, &proc->mm->mmap, vm_sibling)
	{
		if (!iter->vm_file)
			vmm_unmap_range(proc->pdir, iter->vm_start, iter->vm_end);

		list_del(&iter->vm_sibling);
		kfree(iter);
	}
}

static void exit_files(struct process *proc)
{
	for (int i = 0; i < MAX_FD; ++i)
	{
		struct vfs_file *file = proc->files->fd[i];

		if (file && atomic_read(&file->f_count) == 1 && file->f_op->release)
		{
			file->f_op->release(file->f_dentry->d_inode, file);
			kfree(file);
		}
	}
}

static void exit_thread(struct process *proc)
{
	struct thread *th = proc->thread;

	update_thread(th, THREAD_TERMINATED);
	del_timer(&th->sleep_timer);
	vmm_unmap_range(proc->pdir, th->user_stack - STACK_SIZE, th->user_stack);
}

static void exit_notify(struct process *proc)
{
	struct process *iter;
	list_for_each_entry(iter, &proc->children, sibling)
	{
		iter->parent = find_process_by_pid(INIT_PID);
	}
	if (!proc->caused_signal)
		proc->flags |= EXIT_TERMINATED;

	if (proc->pid == proc->sid && proc->tty && proc->pid == proc->tty->session)
	{
		proc->tty->session = 0;
		proc->tty->pgrp = 0;
		do_kill(-proc->tty->pgrp, SIGHUP);
	}

	do_kill(proc->parent->pid, SIGCHLD);
	wake_up(&proc->parent->wait_chld);
}

void do_exit(int32_t code)
{
	lock_scheduler();

	exit_mm(current_process);
	exit_files(current_process);
	exit_thread(current_process);

	current_process->exit_code = code;
	exit_notify(current_process);

	debug_println(DEBUG_INFO, "Process: Exit %d", current_process->pid);
	unlock_scheduler();

	schedule();
}

int32_t do_wait(idtype_t idtype, id_t id, struct infop *infop, int options)
{
	int32_t ret = -1;
	DEFINE_WAIT(wait);
	list_add_tail(&wait.sibling, &current_process->wait_chld.list);

	struct process *pchild = NULL;
	while (true)
	{
		struct process *iter;
		list_for_each_entry(iter, &current_process->children, sibling)
		{
			if (!((idtype == P_PID && iter->pid == id) ||
				  (idtype == P_PGID && iter->gid == id) ||
				  idtype == P_ALL))
				continue;

			if ((options & WEXITED && (iter->flags & SIGNAL_TERMINATED || iter->flags & EXIT_TERMINATED)) ||
				(options & WSTOPPED && iter->flags & SIGNAL_STOPED) ||
				(options & WCONTINUED && iter->flags & SIGNAL_CONTINUED))
			{
				pchild = iter;
				break;
			}
		}

		if (pchild)
			break;

		update_thread(current_thread, THREAD_WAITING);
		schedule();
	}
	list_del(&wait.sibling);

	if (pchild)
	{
		infop->si_pid = pchild->pid;
		infop->si_signo = SIGCHLD;
		infop->si_status = pchild->caused_signal;
		if (pchild->flags & SIGNAL_STOPED)
			infop->si_code = CLD_STOPPED;
		else if (pchild->flags & SIGNAL_CONTINUED)
			infop->si_code = CLD_CONTINUED;
		else if (pchild->flags & SIGNAL_TERMINATED)
			infop->si_code = sig_kernel_coredump(pchild->caused_signal) ? CLD_DUMPED : CLD_KILLED;
		else if (pchild->flags & EXIT_TERMINATED)
		{
			infop->si_code = CLD_EXITED;
			infop->si_status = pchild->exit_code;
		}
		ret = 0;
	}
	debug_println(DEBUG_INFO, "Process: Waiting done %d", current_process->pid);

	return ret;
}

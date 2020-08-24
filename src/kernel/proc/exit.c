#include <include/atomic.h>

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

		if (file && atomic_read(&file->f_count) == 1)
		{
			file->f_op->release(file->f_dentry->d_inode, file);
			kfree(file);
		}
	}
}

static void exit_thread(struct process *proc)
{
	struct thread *tsk = proc->thread;

	update_thread(tsk, THREAD_TERMINATED);
	del_timer(&tsk->sleep_timer);
	vmm_unmap_range(proc->pdir, tsk->user_stack - STACK_SIZE, tsk->user_stack);
}

static void exit_notify(struct process *proc)
{
	struct process *iter;
	list_for_each_entry(iter, &proc->children, sibling)
	{
		do_kill(iter->pid, SIGHUP);
		iter->parent = find_process_by_pid(INIT_PID);
	}
	do_kill(proc->parent->pid, SIGCHLD);
	wake_up(&proc->parent->wait_chld);
}

void do_exit(int32_t code)
{
	exit_mm(current_process);
	exit_files(current_process);
	exit_thread(current_process);

	current_process->exit_code = code;
	exit_notify(current_process);

	schedule();
}

int32_t do_wait(idtype_t idtype, id_t id, struct infop *infop, int options)
{
	int32_t ret = -1;
	DEFINE_WAIT(wentry);

	list_add_tail(&wentry.sibling, &current_process->wait_chld.list);

	struct process *pchild;
	while (true)
	{
		struct process *iter;
		list_for_each_entry(iter, &current_process->children, sibling)
		{
			if (!((idtype == P_PID && iter->pid == id) ||
				  (idtype == P_PGID && iter->gid == id) ||
				  idtype == P_ALL))
				continue;

			if ((options & WEXITED && iter->flags & SIGNAL_TERMINATED) ||
				(options & WSTOPPED && iter->flags & SIGNAL_STOPED) ||
				(options & WCONTINUED && iter->flags & SIGNAL_CONTINUED))
			{
				pchild = iter;
				break;
			}
		}
	}
	list_del(&wentry.sibling);

	if (pchild)
	{
		infop->si_pid = pchild->pid;
		infop->si_signo = pchild->caused_signal;
		infop->si_status = pchild->exit_code;
		if (pchild->flags & SIGNAL_STOPED)
			infop->si_code = CLD_STOPPED;
		else if (pchild->flags & SIGNAL_CONTINUED)
			infop->si_code = CLD_CONTINUED;
		else if (pchild->flags & SIGNAL_TERMINATED)
			infop->si_code = CLD_KILLED;
		else
			infop->si_code = CLD_EXITED;
		ret = 0;
	}

	return ret;
}

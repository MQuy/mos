#include "task.h"

#include <kernel/cpu/hal.h>
#include <kernel/cpu/idt.h>
#include <kernel/cpu/pic.h>
#include <kernel/cpu/tss.h>
#include <kernel/fs/vfs.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/elf.h>
#include <kernel/system/time.h>
#include <kernel/utils/hashmap.h>
#include <kernel/utils/printf.h>
#include <kernel/utils/string.h>

extern void enter_usermode(uint32_t eip, uint32_t esp, uint32_t failed_address);
extern void return_usermode(struct interrupt_registers *regs);
extern int32_t irq_schedule_handler(struct interrupt_registers *regs);
extern int32_t thread_page_fault(struct interrupt_registers *regs);

static uint32_t next_pid = 0;
static uint32_t next_tid = 0;
volatile struct thread *current_thread = NULL;
volatile struct process *current_process = NULL;
volatile struct hashmap *mprocess = NULL;

struct process *find_process_by_pid(pid_t pid)
{
	return hashmap_get(mprocess, &pid);
}

struct files_struct *clone_file_descriptor_table(struct process *parent)
{
	struct files_struct *files = kcalloc(1, sizeof(struct files_struct));

	if (parent)
	{
		memcpy(files, parent->files, sizeof(struct files_struct));
		// NOTE: MQ 2019-12-30 Increasing file description usage when forking because child refers to the same one
		for (uint32_t i = 0; i < MAX_FD; ++i)
			if (parent->files->fd[i])
				atomic_inc(&parent->files->fd[i]->f_count);
	}
	sema_init(&files->lock, 1);
	return files;
}

struct mm_struct *clone_mm_struct(struct process *parent)
{
	struct mm_struct *mm = kcalloc(1, sizeof(struct mm_struct));
	memcpy(mm, parent->mm, sizeof(struct mm_struct));
	INIT_LIST_HEAD(&mm->mmap);

	struct vm_area_struct *iter = NULL;
	list_for_each_entry(iter, &parent->mm->mmap, vm_sibling)
	{
		struct vm_area_struct *clone = kcalloc(1, sizeof(struct vm_area_struct));
		clone->vm_start = iter->vm_start;
		clone->vm_end = iter->vm_end;
		clone->vm_file = iter->vm_file;
		clone->vm_flags = iter->vm_flags;
		clone->vm_mm = mm;
		list_add_tail(&clone->vm_sibling, &mm->mmap);
	}

	return mm;
}

void kernel_thread_entry(struct thread *t, void *flow())
{
	flow();
	schedule();
}

void thread_sleep_timer(struct timer_list *timer)
{
	struct thread *th = from_timer(th, timer, sleep_timer);
	list_del(&timer->sibling);
	update_thread(th, THREAD_READY);
}

void thread_sleep(uint32_t ms)
{
	mod_timer(&current_thread->sleep_timer, get_milliseconds(NULL) + ms);
	update_thread(current_thread, THREAD_WAITING);
	schedule();
}

struct thread *create_kernel_thread(struct process *parent, uint32_t eip, enum thread_state state, int priority)
{
	lock_scheduler();

	struct thread *th = kcalloc(1, sizeof(struct thread));
	th->tid = next_tid++;
	th->kernel_stack = (uint32_t)(kcalloc(STACK_SIZE, sizeof(char)) + STACK_SIZE);
	th->parent = parent;
	th->state = state;
	th->policy = THREAD_KERNEL_POLICY;
	th->esp = th->kernel_stack - sizeof(struct trap_frame);
	plist_node_init(&th->sched_sibling, priority);
	th->sleep_timer = (struct timer_list)TIMER_INITIALIZER(thread_sleep_timer, UINT32_MAX);

	struct trap_frame *frame = (struct trap_frame *)th->esp;
	memset(frame, 0, sizeof(struct trap_frame));

	frame->parameter2 = eip;
	frame->parameter1 = (uint32_t)th;
	frame->return_address = PROCESS_TRAPPED_PAGE_FAULT;
	frame->eip = (uint32_t)kernel_thread_entry;

	frame->eax = 0;
	frame->ecx = 0;
	frame->edx = 0;
	frame->ebx = 0;
	frame->esp = 0;
	frame->ebp = 0;
	frame->esi = 0;
	frame->edi = 0;

	parent->thread = th;

	unlock_scheduler();

	return th;
}

struct process *create_process(struct process *parent, const char *name, struct pdirectory *pdir)
{
	lock_scheduler();

	struct process *proc = kcalloc(1, sizeof(struct process));
	proc->pid = next_pid++;
	proc->name = strdup(name);
	if (pdir)
		proc->pdir = vmm_create_address_space(pdir);
	else
		proc->pdir = vmm_get_directory();
	proc->parent = parent;
	proc->files = clone_file_descriptor_table(parent);
	proc->fs = kcalloc(1, sizeof(struct fs_struct));
	proc->mm = kcalloc(1, sizeof(struct mm_struct));
	INIT_LIST_HEAD(&proc->wait_chld.list);
	INIT_LIST_HEAD(&proc->mm->mmap);

	for (int i = 0; i < NSIG; ++i)
		proc->sighand[i].sa_handler = sig_kernel_ignore(i + 1) ? SIG_IGN : SIG_DFL;

	if (parent)
	{
		proc->gid = parent->gid;
		proc->sid = parent->sid;
		memcpy(proc->fs, parent->fs, sizeof(struct fs_struct));
		list_add_tail(&proc->sibling, &parent->children);
	}

	INIT_LIST_HEAD(&proc->children);

	hashmap_put(mprocess, &proc->pid, proc);

	unlock_scheduler();

	return proc;
}

void setup_swapper_process()
{
	current_process = create_process(NULL, "swapper", NULL);
	current_thread = create_kernel_thread(current_process, 0, THREAD_RUNNING, 0);
}

struct process *create_kernel_process(const char *pname, void *func, int32_t priority)
{
	struct process *proc = create_process(current_process, pname, current_process->pdir);
	create_kernel_thread(proc, (uint32_t)func, THREAD_WAITING, priority);

	return proc;
}

void task_init(void *func)
{
	DEBUG &&debug_println(DEBUG_INFO, "[task] - Initializing");

	mprocess = kcalloc(1, sizeof(struct hashmap));
	hashmap_init(mprocess, hashmap_hash_uint32, hashmap_compare_uint32, 0);
	sched_init();
	register_interrupt_handler(IRQ8, irq_schedule_handler);
	register_interrupt_handler(14, thread_page_fault);

	DEBUG &&debug_println(DEBUG_INFO, "\tSetup swapper process");
	setup_swapper_process();

	DEBUG &&debug_println(DEBUG_INFO, "\tSetup init process");
	struct process *init = create_process(current_process, "init", current_process->pdir);
	init->gid = init->pid;
	init->sid = init->pid;

	struct thread *nt = create_kernel_thread(init, (uint32_t)func, THREAD_WAITING, 1);
	update_thread(current_thread, THREAD_TERMINATED);
	update_thread(nt, THREAD_READY);

	DEBUG &&debug_println(DEBUG_INFO, "[task] - Done");
	schedule();
}

void user_thread_entry(struct thread *th)
{
	// explain in kernel_init#unlock_scheduler
	unlock_scheduler();

	tss_set_stack(0x10, th->kernel_stack);
	return_usermode(&th->uregs);
}

void user_thread_elf_entry(struct thread *th, const char *path, void (*setup)(struct Elf32_Layout *))
{
	// explain in kernel_init#unlock_scheduler
	unlock_scheduler();

	char *buf = vfs_read(path);
	struct Elf32_Layout *elf_layout = elf_load(buf);
	th->user_stack = elf_layout->stack;
	tss_set_stack(0x10, th->kernel_stack);
	if (setup)
		setup(elf_layout);
	enter_usermode(elf_layout->stack, elf_layout->entry, PROCESS_TRAPPED_PAGE_FAULT);
}

struct thread *create_user_thread(struct process *parent, const char *path, enum thread_state state, enum thread_policy policy, int priority, void (*setup)(struct Elf32_Layout *))
{
	lock_scheduler();

	struct thread *th = kcalloc(1, sizeof(struct thread));
	th->tid = next_tid++;
	th->parent = parent;
	th->state = state;
	th->policy = policy;
	th->kernel_stack = (uint32_t)(kcalloc(STACK_SIZE, sizeof(char)) + STACK_SIZE);
	th->esp = th->kernel_stack - sizeof(struct trap_frame);
	plist_node_init(&th->sched_sibling, priority);
	th->sleep_timer = (struct timer_list)TIMER_INITIALIZER(thread_sleep_timer, UINT32_MAX);

	struct trap_frame *frame = (struct trap_frame *)th->esp;
	memset(frame, 0, sizeof(struct trap_frame));

	frame->parameter3 = (uint32_t)setup;
	frame->parameter2 = (uint32_t)strdup(path);
	frame->parameter1 = (uint32_t)th;
	frame->return_address = PROCESS_TRAPPED_PAGE_FAULT;
	frame->eip = (uint32_t)user_thread_elf_entry;

	frame->eax = 0;
	frame->ecx = 0;
	frame->edx = 0;
	frame->ebx = 0;
	frame->esp = 0;
	frame->ebp = 0;
	frame->esi = 0;
	frame->edi = 0;

	parent->thread = th;

	unlock_scheduler();

	return th;
}

void process_load(const char *pname, const char *path, enum thread_policy policy, int priority, void (*setup)(struct Elf32_Layout *))
{
	struct process *proc = create_process(current_process, pname, current_process->pdir);
	struct thread *th = create_user_thread(proc, path, THREAD_READY, policy, priority, setup);
	queue_thread(th);
}

struct process *process_fork(struct process *parent)
{
	lock_scheduler();

	// fork process
	struct process *proc = kcalloc(1, sizeof(struct process));
	proc->pid = next_pid++;
	proc->gid = parent->pid;
	proc->sid = parent->sid;
	proc->name = strdup(parent->name);
	proc->parent = parent;
	proc->mm = clone_mm_struct(parent);
	memcpy(&proc->sighand, &parent->sighand, sizeof(parent->sighand));

	INIT_LIST_HEAD(&proc->children);

	list_add_tail(&proc->sibling, &parent->children);

	proc->fs = kcalloc(1, sizeof(struct fs_struct));
	memcpy(proc->fs, parent->fs, sizeof(struct fs_struct));

	proc->files = clone_file_descriptor_table(parent);
	proc->pdir = vmm_fork(parent->pdir);

	// copy active parent's thread
	struct thread *parent_thread = parent->thread;
	struct thread *th = kcalloc(1, sizeof(struct thread));
	th->tid = next_tid++;
	th->state = THREAD_READY;
	th->policy = THREAD_APP_POLICY;
	th->time_slice = 0;
	th->parent = proc;
	th->kernel_stack = (uint32_t)(kcalloc(STACK_SIZE, sizeof(char)) + STACK_SIZE);
	th->user_stack = parent_thread->user_stack;
	// NOTE: MQ 2019-12-18 Setup trap frame
	th->esp = th->kernel_stack - sizeof(struct trap_frame);
	plist_node_init(&th->sched_sibling, parent_thread->sched_sibling.prio);

	memcpy(&th->uregs, &parent_thread->uregs, sizeof(struct interrupt_registers));
	th->uregs.eax = 0;

	struct trap_frame *frame = (struct trap_frame *)th->esp;
	frame->parameter1 = (uint32_t)th;
	frame->return_address = PROCESS_TRAPPED_PAGE_FAULT;
	frame->eip = (uint32_t)user_thread_entry;

	frame->eax = 0;
	frame->ecx = 0;
	frame->edx = 0;
	frame->ebx = 0;
	frame->esp = 0;
	frame->ebp = 0;
	frame->esi = 0;
	frame->edi = 0;

	proc->thread = th;
	hashmap_put(mprocess, &proc->pid, proc);

	unlock_scheduler();

	return proc;
}

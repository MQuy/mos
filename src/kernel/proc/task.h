#ifndef PROC_TASK_H
#define PROC_TASK_H

#include <include/ctype.h>
#include <include/list.h>
#include <kernel/cpu/idt.h>
#include <kernel/locking/semaphore.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/elf.h>
#include <kernel/system/timer.h>
#include <kernel/utils/plist.h>
#include <stdint.h>

#define MAX_FD 256
#define PROCESS_TRAPPED_PAGE_FAULT 0xFFFFFFFF
#define MAX_THREADS 0x10000
#define STACK_SIZE 0x2000
#define UHEAP_SIZE 0x20000

// vm_flags
#define VM_READ 0x00000001 /* currently active flags */
#define VM_WRITE 0x00000002
#define VM_EXEC 0x00000004
#define VM_SHARED 0x00000008

struct vfs_file;
struct vfs_dentry;
struct vfs_mount;

enum thread_state
{
	THREAD_NEW,
	THREAD_READY,
	THREAD_RUNNING,
	THREAD_WAITING,
	THREAD_TERMINATED,
} thread_state;

enum thread_policy
{
	THREAD_KERNEL_POLICY,
	THREAD_SYSTEM_POLICY,
	THREAD_APP_POLICY,
} thread_policy;

struct files_struct
{
	struct semaphore lock;
	struct vfs_file *fd[MAX_FD];
};

struct fs_struct
{
	struct vfs_dentry *d_root;
	struct vfs_mount *mnt_root;
};

struct trap_frame
{
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  // Pushed by pusha.
	uint32_t eip;									  // eip is saved on stack by the caller's "CALL" instruction
	uint32_t return_address;
	uint32_t parameter1, parameter2, parameter3;
};

struct vm_area_struct
{
	struct mm_struct *vm_mm;
	uint32_t vm_start;
	uint32_t vm_end;
	uint32_t vm_flags;

	struct list_head vm_sibling;
	struct vfs_file *vm_file;
};

struct mm_struct
{
	struct list_head mmap;
	uint32_t free_area_cache;
	uint32_t start_code, end_code, start_data, end_data;
	// NOTE: MQ 2020-01-30
	// end_brk is marked as the end of heap section, brk is end but in range start_brk<->end_brk and expand later
	// better way is only mapping start_brk->brk and handling page fault brk->end_brk
	uint32_t start_brk, brk, end_brk, start_stack;
};

struct thread
{
	tid_t tid;
	enum thread_state state;
	enum thread_policy policy;
	struct process *parent;
	uint32_t esp;
	uint32_t kernel_stack;
	uint32_t user_stack;
	uint32_t expiry_when;
	uint32_t time_slice;
	int32_t exit_code;
	struct interrupt_registers uregs;
	struct list_head sibling;
	struct plist_node sched_sibling;
	struct timer_list sleep_timer;
};

struct process
{
	pid_t pid;
	uid_t uid;
	gid_t gid;
	char *name;
	struct pdirectory *pdir;
	struct fs_struct *fs;
	struct files_struct *files;
	struct mm_struct *mm;
	struct process *parent;
	struct thread *active_thread;
	struct list_head sibling;
	struct list_head children;
	struct list_head threads;
};

void task_init();
void sched_init();

struct thread *create_kernel_thread(struct process *parent, uint32_t eip, enum thread_state state, int priority);
struct thread *create_user_thread(struct process *parent, const char *path, enum thread_state state, enum thread_policy policy, int priority, void (*setup)(struct Elf32_Layout *));
void update_thread(struct thread *thread, uint8_t state);
struct process *create_kernel_process(const char *pname, void *func, int32_t priority);
void process_load(const char *pname, const char *path, int priority, void (*setup)(struct Elf32_Layout *));
struct process *process_fork(struct process *parent);
void queue_thread(struct thread *t);
void switch_thread(struct thread *nt);
void schedule();
struct plist_head *get_list_from_thread(enum thread_state state, enum thread_policy policy);
int get_top_priority_from_list(enum thread_state state, enum thread_policy policy);
void thread_sleep(uint32_t ms);

#endif

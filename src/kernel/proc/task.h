#ifndef PROC_TASK_H
#define PROC_TASK_H

#include <cpu/idt.h>
#include <ipc/signal.h>
#include <locking/semaphore.h>
#include <memory/vmm.h>
#include <proc/elf.h>
#include <shared/ctype.h>
#include <shared/list.h>
#include <stdint.h>
#include <system/timer.h>
#include <utils/hashmap.h>
#include <utils/plist.h>

#define SWAPPER_PID 0
#define INIT_PID 1

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

#define SIGNAL_STOPED 0x01
#define SIGNAL_CONTINUED 0x02
#define SIGNAL_TERMINATED 0x04
#define EXIT_TERMINATED 0x08

struct vfs_file;
struct vfs_dentry;
struct vfs_mount;
struct tty_struct;

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
	int32_t priority;  // input priority, `sched_sibling.prio` is adjusted number based on scheduler
	struct process *parent;

	uint32_t esp;
	uint32_t kernel_stack;
	uint32_t user_stack;
	struct interrupt_registers uregs;

	sigset_t pending;
	sigset_t blocked;
	bool signaling;

	uint32_t time_slice;

	struct plist_node sched_sibling;
	struct timer_list sleep_timer;
};

struct process
{
	pid_t pid;
	gid_t gid;
	sid_t sid;

	char *name;
	struct process *parent;
	struct thread *thread;
	struct pdirectory *pdir;

	struct fs_struct *fs;
	struct files_struct *files;
	struct mm_struct *mm;

	struct tty_struct *tty;
	struct sigaction sighand[NSIG];
	int32_t exit_code;
	int32_t caused_signal;
	uint32_t flags;
	struct wait_queue_head wait_chld;

	struct list_head sibling;
	struct list_head children;
};

extern volatile struct thread *current_thread;
extern volatile struct process *current_process;
extern volatile struct hashmap *mprocess;

#define for_each_process(p)         \
	struct hashmap_iter *__hm_iter; \
	for (__hm_iter = hashmap_iter(mprocess), p = hashmap_iter_get_data(__hm_iter); __hm_iter; __hm_iter = hashmap_iter_next(mprocess, __hm_iter), p = hashmap_iter_get_data(__hm_iter))

// task.c
void task_init();
struct process *create_system_process(const char *pname, void *func, int32_t priority);
void process_load(const char *pname, const char *path, enum thread_policy policy, int priority, void (*setup)(struct Elf32_Layout *));
struct process *process_fork(struct process *parent);
int32_t process_execve(const char *pathname, char *const argv[], char *const envp[]);
void thread_sleep(uint32_t ms);
struct process *find_process_by_pid(pid_t pid);

// sched.c
void update_thread(struct thread *thread, uint8_t state);
void queue_thread(struct thread *t);
void schedule();
void sched_init();
void lock_scheduler();
void unlock_scheduler();
int get_top_priority_from_list(enum thread_state state, enum thread_policy policy);
void wake_up(struct wait_queue_head *hq);
int32_t thread_page_fault(struct interrupt_registers *regs);
int32_t irq_schedule_handler(struct interrupt_registers *regs);

// exit.c
int32_t do_wait(idtype_t idtype, id_t id, struct infop *infop, int options);
void do_exit(int32_t code);

#endif

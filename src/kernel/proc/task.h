#ifndef PROC_TASK_H
#define PROC_TASK_H

#include <stdint.h>
#include <include/ctype.h>
#include <include/list.h>
#include <kernel/cpu/idt.h>
#include <kernel/utils/plist.h>
#include <kernel/locking/semaphore.h>
#include <kernel/memory/vmm.h>

#define MAX_FD 256
#define PROCESS_TRAPPED_PAGE_FAULT 0xFFFFFFFF

struct vfs_file;
struct vfs_dentry;
struct vfs_mount;

typedef enum thread_state
{
  NEW,
  READY_TO_RUN,
  RUNNING,
  WAITING,
  TERMINATED,
} thread_state;

typedef enum thread_policy
{
  KERNEL_POLICY,
  SYSTEM_POLICY,
  APP_POLICY
} thread_policy;

typedef struct files_struct
{
  semaphore lock;
  struct vfs_file *fd[MAX_FD];
} files_struct;

typedef struct fs_struct
{
  struct vfs_dentry *d_root;
  struct vfs_mount *mnt_root;
} fs_struct;

typedef struct trap_frame
{
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
  uint32_t eip;                                    // eip is saved on stack by the caller's "CALL" instruction
  uint32_t return_address;
  uint32_t parameter1, parameter2;
} trap_frame;

typedef struct thread
{
  tid_t tid;
  enum thread_state state;
  struct process *parent;
  uint32_t esp;
  uint32_t kernel_stack;
  uint32_t user_stack;
  uint32_t expiry_when;
  uint32_t time_used;
  int32_t exit_code;
  struct interrupt_registers uregs;
  struct list_head sibling;
  struct plist_node sched_sibling;
} thread;

typedef struct process
{
  pid_t pid;
  uid_t uid;
  gid_t gid;
  char *name;
  struct pdirectory *pdir;
  struct fs_struct *fs;
  struct files_struct *files;
  struct process *parent;
  struct thread *active_thread;
  struct list_head sibling;
  struct list_head children;
  struct list_head threads;
} process;

typedef struct process_image
{
  uint32_t eip;
  uint32_t stack;
} process_image;

void task_init();
void sched_init();

thread *create_kernel_thread(process *parent, uint32_t eip, thread_state state, int priority);
thread *create_user_thread(process *parent, const char *path, thread_state state, thread_policy policy, int priority);
void update_thread(thread *thread, uint8_t state);
process *create_process(process *parent, const char *name, pdirectory *pdir);
void process_load(const char *pname, const char *path);
process *process_fork(process *parent);
void queue_thread(thread *t);
void switch_thread(thread *nt);
void schedule();

#endif
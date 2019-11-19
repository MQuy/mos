#ifndef SYSTEM_TASK_H
#define SYSTEM_TASK_H

#include <stdint.h>
#include <kernel/include/ctype.h>
#include <kernel/include/list.h>
#include <kernel/memory/vmm.h>

struct fs_struct;
struct files_struct;

enum thread_state
{
  RUNNING,
  READY_TO_RUN,
  WAITING,
  TERMINATED,
};
enum thread_policy
{
  KERNEL_POLICY,
  SYSTEM_POLICY,
  APP_POLICY
};

typedef struct trap_frame
{
  uint32_t gs, fs, es, ds;                         // Data segment selector
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
  uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically.
} trap_frame;

typedef struct thread
{
  tid_t tid;
  enum thread_state state;
  struct process *parent;
  uint32_t priority;
  uint32_t policy;
  uint32_t esp;
  uint32_t kernel_esp;
  uint32_t expiry_when;
  uint32_t time_used;
  struct list_head sibling;
} thread;

typedef struct process
{
  pid_t pid;
  char *name;
  struct pdirectory *pdir;
  struct fs_struct *fs;
  struct files_struct *files;
  struct process *parent;
  struct list_head sibling;
  struct list_head children;
  struct list_head threads;
  uid_t uid;
  gid_t gid;
} process;

void task_init();
void task_start();
void task_schedule(uint32_t esp);

thread *create_thread(uint32_t eip, uint32_t esp, bool in_kernel);
void block_thread(thread *thread, uint8_t state);
void schedule();
process *create_process(process *parent, uint32_t eip, uint32_t esp, bool is_kernel);

bool queue_push(thread t);

#endif
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
  NEW,
  READY_TO_RUN,
  RUNNING,
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
  uint32_t priority;
  uint32_t policy;
  uint32_t esp;
  uint32_t kernel_stack;
  uint32_t user_stack;
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

typedef struct process_image
{
  uint32_t eip;
  uint32_t stack;
} process_image;

void task_init();

thread *create_kernel_thread(process *parent, uint32_t eip, enum thread_state state);
void block_thread(thread *thread, uint8_t state);
process *create_process(process *parent, const char *name, pdirectory *pdir);
void process_load(const char *pname, const char *path);
void terminate_thread();
void queue_thread(thread *t);

void schedule();

#endif
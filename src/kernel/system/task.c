#include "../cpu/idt.h"
#include "../cpu/pic.h"
#include "../cpu/pit.h"
#include "../memory/pmm.h"
#include "../memory/vmm.h"
#include "task.h"

#define KERNEL_STACK_ALLOC_BASE 0xe0000000
#define MAX_THREAD 6

extern void irq_task_handler();

thread _ready_queue[MAX_THREAD];
int _queue_first = 0, _queue_last = 0, _queue_current;
int _kernel_stack_index = 0;

uint32_t create_kernel_stack()
{
  physical_addr pAddr;
  virtual_addr vAddr;

  pAddr = pmm_alloc_block();

  if (!pAddr)
    return 0;

  vAddr = KERNEL_STACK_ALLOC_BASE + _kernel_stack_index * PMM_FRAME_SIZE;
  vmm_map_phyiscal_address(vmm_get_directory(), vAddr, pAddr, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);

  _kernel_stack_index++;

  return vAddr + PMM_FRAME_SIZE;
}

thread create_thread(void *func, uint32_t esp)
{
#define KERNEL_DATA 0x10
#define KERNEL_CODE 0x8
  esp = esp - sizeof(trap_frame);

  thread t;
  trap_frame *frame = (trap_frame *)esp;
  memcpy(frame, 0, sizeof(trap_frame));

  frame->eflags = 0x202;
  frame->cs = 0x8;
  frame->eip = (uint32_t)func;

  frame->eax = 0;
  frame->ecx = 0;
  frame->edx = 0;
  frame->ebx = 0;
  frame->esp = 0;
  frame->ebp = 0;
  frame->esi = 0;
  frame->edi = 0;

  frame->ds = 0x10;
  frame->es = 0x10;
  frame->fs = 0x10;
  frame->gs = 0x10;

  t.esp = esp;

  return t;
}

bool queue_push(thread t)
{
  _ready_queue[_queue_last % MAX_THREAD] = t;
  _queue_last++;
  return true;
}

void clear_queue()
{
  _queue_first = _queue_last = 0;
}

thread *get_current_thread()
{
  return &_ready_queue[_queue_current];
}

void task_init()
{
  clear_queue();
  setvect(32, irq_task_handler);
}

void task_start()
{

  if (_queue_last > 0)
  {
    _queue_current = 0;
    thread *t = get_current_thread();
    __asm__ __volatile__("mov %0, %%esp \n"
                         "pop %%gs      \n"
                         "pop %%fs      \n"
                         "pop %%es      \n"
                         "pop %%ds      \n"
                         "popa          \n"
                         "iret          \n" ::"r"(t->esp));
  }
}

/*
NOTE: MQ 2019-05-02
The reason EOI not called in this interrupt because the default isr32
which is setup in pit.c is chained at the bottom of irq_scheduler
*/
uint32_t task_schedule(uint32_t esp)
{
  if (_queue_last == 0)
    return 0;

  thread *current_thread = get_current_thread();
  current_thread->esp = esp;

  _queue_current = (_queue_current + 1) % _queue_last;
  thread *next_thread = get_current_thread();

  return next_thread->esp;
}

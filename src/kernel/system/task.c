#include <kernel/cpu/idt.h>
#include <kernel/cpu/pic.h>
#include <kernel/cpu/pit.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include "task.h"

#define MAX_THREAD 6

extern void irq_task_handler();

thread _ready_queue[MAX_THREAD];
int _queue_first, _queue_last, _queue_current;
bool is_task_running;

thread create_thread(void *func, uint32_t esp)
{
#define USER_DATA 0x23
#define USER_CODE 0x1B
  esp = esp - sizeof(trap_frame);

  thread t;
  trap_frame *frame = (trap_frame *)esp;
  memcpy(frame, 0, sizeof(trap_frame));

  frame->eflags = 0x202;
  frame->cs = USER_CODE;
  frame->eip = (uint32_t)func;
  frame->useresp = esp + sizeof(trap_frame);
  frame->ss = USER_DATA;

  frame->eax = 0;
  frame->ecx = 0;
  frame->edx = 0;
  frame->ebx = 0;
  frame->esp = 0;
  frame->ebp = 0;
  frame->esi = 0;
  frame->edi = 0;

  frame->ds = USER_DATA;
  frame->es = USER_DATA;
  frame->fs = USER_DATA;
  frame->gs = USER_DATA;

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
  _queue_first = 0;
  _queue_last = 0;
  is_task_running = false;
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
    _queue_first = 0;
    _queue_current = 0;
    is_task_running = true;
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
void task_schedule(uint32_t esp)
{
  if (_queue_last == 0 || is_task_running == false)
    return 0;

  thread *current_thread = get_current_thread();
  /*
    NOTE: MQ 2019-05-05
    When the isr32 is triggered, cpu pushs eip, cs, eflags, useresp, ss (in usermode)
    and we also push registers and segements (in irq_task_handler).
    We get user's stack (15th), add padding for trap frame -> copying value in current stack (tss) to this padding
    so we can restore when switching back via isr32
  */
  uint32_t ret = *(uint32_t *)(esp + 15 * 4);
  current_thread->esp = ret - sizeof(trap_frame);
  memcpy(current_thread->esp, esp, sizeof(trap_frame));

  _queue_current = (_queue_current + 1) % _queue_last;
  thread *next_thread = get_current_thread();

  memcpy(esp, next_thread->esp, sizeof(trap_frame));
}

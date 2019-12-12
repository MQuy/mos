#include <include/list.h>
#include <kernel/memory/malloc.h>
#include "idt.h"
#include "pic.h"
#include "pit.h"

#define PIT_REG_COUNTER 0x40
#define PIT_REG_COMMAND 0x43
#define TICKS_PER_SECOND 1000

volatile uint32_t pit_ticks = 0;

typedef struct pit_handler
{
  uint32_t handler;
  struct list_head sibling;
} pit_handler;

struct list_head handler_list;

void pit_interrupt_handler(interrupt_registers *regs)
{
  pit_ticks++;

  pit_handler *pt = NULL;
  list_for_each_entry(pt, &handler_list, sibling)
  {
    if (pt)
    {
      I86_IRQ_HANDLER handler = pt->handler;
      handler(regs);
    }
  }
}

uint32_t get_milliseconds_from_boot()
{
  return pit_ticks * TICKS_PER_SECOND;
}

void register_pit_handler(I86_IRQ_HANDLER handler)
{
  pit_handler *pt = calloc(1, sizeof(pit_handler));
  pt->handler = handler;
  list_add_tail(&pt->sibling, &handler_list);
}

void pit_init()
{
  int divisor = 1193181 / TICKS_PER_SECOND;

  outportb(PIT_REG_COMMAND, 0x34);
  outportb(PIT_REG_COUNTER, divisor & 0xff);
  outportb(PIT_REG_COUNTER, (divisor >> 8) & 0xff);

  register_interrupt_handler(IRQ0, pit_interrupt_handler);

  INIT_LIST_HEAD(&handler_list);
}
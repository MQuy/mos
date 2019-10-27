#include "idt.h"
#include "pic.h"
#include "pit.h"

#define PIT_REG_COUNTER 0x40
#define PIT_REG_COMMAND 0x43

volatile uint32_t pit_ticks = 0;

// FIXME: MQ 2019-09-26 Use single linked list instead of fixed array
#define PIT_MAX_HANDLER 10
I86_IRQ_HANDLER *pit_handlers[PIT_MAX_HANDLER];

void pit_interrupt_handler(interrupt_registers *regs)
{
  pit_ticks++;

  // for (uint8_t i = 0; i < PIT_MAX_HANDLER; ++i)
  //   if (pit_handlers[i])
  //   {
  //     I86_IRQ_HANDLER handler = pit_handlers[i];
  //     handler(regs);
  //   }
}

uint32_t get_milliseconds_from_boot()
{
  return pit_ticks * 1000;
}

void register_pit_handler(I86_IRQ_HANDLER handler)
{
  for (uint8_t i = 0; i < PIT_MAX_HANDLER; ++i)
  {
    if (!pit_handlers[i])
    {
      pit_handlers[i] = handler;
      return;
    }
  }
}

void pit_init()
{
  int divisor = 1193181 / 1000;

  outportb(PIT_REG_COMMAND, 0x34);
  outportb(PIT_REG_COUNTER, divisor & 0xff);
  outportb(PIT_REG_COUNTER, (divisor >> 8) & 0xff);

  register_interrupt_handler(IRQ0, pit_interrupt_handler);
}
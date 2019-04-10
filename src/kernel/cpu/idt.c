#include "idt.h"
#include "../graphics/DebugDisplay.h"

static struct idt_descriptor _idt[I86_MAX_INTERRUPTS];
static struct idtr _idtr;

static void idt_install()
{
  __asm__ __volatile__("lidt (%0)" ::"r"(&_idtr));
}

void idt_install_ir(uint8_t i, uint16_t flags, uint16_t sel, I86_IRQ_HANDLER irq)
{
  if (i > I86_MAX_INTERRUPTS)
    return 0;

  uint64_t base = (uint64_t) & (*irq);

  _idt[i].baseLo = base & 0xffff;
  _idt[i].baseHi = (base >> 16) & 0xffff;
  _idt[i].flags = flags;
  _idt[i].sel = sel;
  _idt[i].reserved = 0;
}

void i86_default_handler(interrupt_registers *regs)
{
  uint8_t error_code = regs->err_code & 0xff;

  DebugPrintf("\nUnhandled exception %d", error_code);
}

void idt_init()
{
  _idtr.limit = sizeof(struct idt_descriptor) * I86_MAX_INTERRUPTS - 1;
  _idtr.base = &_idt[0];

  memset(&_idt[0], 0, _idtr.limit);

  for (int i = 0; i < 256; ++i)
    idt_install_ir(i, I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32, 0, i86_default_handler);

  idt_install();
}
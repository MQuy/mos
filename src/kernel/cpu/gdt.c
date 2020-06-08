#include <kernel/utils/string.h>
#include <kernel/utils/printf.h>
#include "gdt.h"

extern void gdt_flush(uint32_t);

static struct gdt_descriptor _gdt[MAX_DESCRIPTORS];
static struct gdtr _gdtr;

void gdt_set_descriptor(uint32_t i, uint64_t base, uint64_t limit, uint8_t access, uint8_t grand)
{
  if (i > MAX_DESCRIPTORS)
    return;

  //! null out the descriptor
  memset((void *)&_gdt[i], 0, sizeof(struct gdt_descriptor));

  //! set limit and base addresses
  _gdt[i].baseLo = (uint16_t)(base & 0xffff);
  _gdt[i].baseMid = (uint8_t)((base >> 16) & 0xff);
  _gdt[i].baseHi = (uint8_t)((base >> 24) & 0xff);
  _gdt[i].limit = (uint16_t)(limit & 0xffff);

  //! set flags and grandularity bytes
  _gdt[i].flags = access;
  _gdt[i].grand = (uint8_t)((limit >> 16) & 0x0f);
  _gdt[i].grand |= grand & 0xf0;
}

void gdt_init()
{
  debug_println(DEBUG_INFO, "[gdt] - Initializing");

  _gdtr.limit = (sizeof(struct gdt_descriptor) * MAX_DESCRIPTORS) - 1;
  _gdtr.base = (uint32_t)&_gdt[0];

  //! set null descriptor
  gdt_set_descriptor(0, 0, 0, 0, 0);

  //! set default code descriptor
  gdt_set_descriptor(1, 0, 0xffffffff,
                     I86_GDT_DESC_READWRITE | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
                     I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

  //! set default data descriptor
  gdt_set_descriptor(2, 0, 0xffffffff,
                     I86_GDT_DESC_READWRITE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
                     I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

  //! set default user mode code descriptor
  gdt_set_descriptor(3, 0, 0xffffffff,
                     I86_GDT_DESC_READWRITE | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_CODEDATA |
                         I86_GDT_DESC_MEMORY | I86_GDT_DESC_DPL,
                     I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

  //! set default user mode data descriptor
  gdt_set_descriptor(4, 0, 0xffffffff,
                     I86_GDT_DESC_READWRITE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY |
                         I86_GDT_DESC_DPL,
                     I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

  gdt_flush((uint32_t)&_gdtr);

  debug_println(DEBUG_INFO, "[gdt] - Done");
}

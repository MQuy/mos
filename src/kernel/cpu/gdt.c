#include "gdt.h"

static struct gdt_descriptor _gdt[MAX_DESCRIPTORS];
static struct gdtr _gdtr;

static void gdt_install()
{
  __asm__ __volatile__("lgdt (%0)" ::"r"(&_gdtr));
}

void gdt_set_descriptor(uint8_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t grand)
{
  if (i > MAX_DESCRIPTORS)
    return;

  memset(&_gdt[i], 0, sizeof(gdt_descriptor));

  _gdt[i].baseLo = base & 0xffff;
  _gdt[i].baseMid = (base >> 16) & 0xff;
  _gdt[i].baseHi = (base >> 24) & 0xff;
  _gdt[i].limit = limit & 0xffff;

  _gdt[i].flags = access;
  _gdt[i].grand = (limit >> 16) & 0x0f;
  _gdt[i].grand |= grand & 0xf0;
}

void gdt_init()
{
  _gdtr.limit = sizeof(struct gdt_descriptor) * MAX_DESCRIPTORS - 1;
  _gdtr.base = (uint32_t)&_gdt[0];

  memset(&_gdt[0], 0, _gdtr.limit);

  // null descriptor
  gdt_set_descriptor(0, 0, 0, 0, 0);
  // code descriptor
  gdt_set_descriptor(1, 0, 0xffffffff, I86_GDT_DESC_READWRITE | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
                     I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);
  // data descriptor
  gdt_set_descriptor(2, 0, 0xffffffff, I86_GDT_DESC_READWRITE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
                     I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

  gdt_install();
}
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "include/multiboot.h"
#include "graphics/DebugDisplay.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"

int kernel_main(uint32_t boot_magic, multiboot_info_t *boot_info)
{

  if (boot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
  {
    DebugPrintf("\nBootloader MAGIC is invalid... Halting");
    return -1;
  }

  DebugClrScr(0x13);
  DebugGotoXY(0, 0);
  DebugSetColor(0x17);

  gdt_init();
  idt_init();

  pmm_init(boot_info);
  vmm_init();

  __asm__ __volatile__("int $5");

  DebugPrintf("\nHalting ...");
  return 0;
}
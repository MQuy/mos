#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "include/multiboot.h"
#include "graphics/DebugDisplay.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"

char *strMemoryTypes[] = {

    "Available",
    "Reserved",
    "ACPI Reclaim",
    "ACPI NVS Memory",
    "Bad Memory"};

int kernel_main(uint32_t boot_magic, multiboot_info_t *boot_info)
{

  if (boot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
  {
    DebugPrintf("Bootloader MAGIC is invalid... Halting\n");
    return -1;
  }

  DebugClrScr(0x13);
  DebugGotoXY(0, 0);
  DebugSetColor(0x17);

  gdt_init();
  idt_init();

  pmm_init(boot_info);
  vmm_init();

  DebugPrintf("\nExit mos");
  return 0;
}
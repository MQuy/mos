#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"
#include "DebugDisplay.h"

char *strMemoryTypes[] = {

    "Available",      //memory_region.type==0
    "Reserved",       //memory_region.type==1
    "ACPI Reclaim",   //memory_region.type==2
    "ACPI NVS Memory" //memory_region.type==3
    "No Such Memory"};

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

  DebugPrintf("mem_lower = %dKB, mem_upper = %dKB\n",
              (unsigned)boot_info->mem_lower, (unsigned)boot_info->mem_upper);
  DebugPrintf("mem map: 0x%x %d", (unsigned)boot_info->mmap_addr, (unsigned)boot_info->mmap_length);

  multiboot_memory_map_t *region = (multiboot_memory_map_t *)boot_info->mmap_addr;

  for (int i = 0; region < boot_info->mmap_addr + boot_info->mmap_length; i++)
  {
    if (region->type > 4 && region->addr == 0)
      break;

    DebugPrintf("\nregion %i: start = 0x%x, length (bytes): 0x%x, type = %d (%s)", i,
                (unsigned)region->addr,
                (unsigned)region->len,
                (unsigned)region->type, strMemoryTypes[(unsigned)region->type - 1]);

    region = (multiboot_memory_map_t *)((unsigned long)region + region->size + sizeof(region->size));
  }
  return 0;
}
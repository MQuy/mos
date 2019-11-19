#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "include/multiboot2.h"
#include "graphics/DebugDisplay.h"
#include "cpu/hal.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/pit.h"
#include "system/tss.h"
#include "system/sysapi.h"
#include "system/exception.h"
#include "system/user.h"
#include "system/time.h"
#include "system/task.h"
#include "devices/kybrd.h"
#include "devices/mouse.h"
#include "devices/pci.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "memory/malloc.h"
#include "devices/ata.h"
#include "fs/vfs.h"
#include "fs/ext2/ext2.h"
#include "system/console.h"

extern vfs_file_system_type ext2_fs_type;

void kernel_init()
{
  // FIXME: MQ 2019-11-19 ata_init is not called in pci_scan_buses without enabling -O2
  // pci_scan_buses();
  ata_init();

  vfs_init(&ext2_fs_type, "/dev/hda");
  // setup stack and enter user mode
  // setup_and_enter_usermode();
}

int kernel_main(unsigned long addr, unsigned long magic)
{
  if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
  {
    return -1;
  }

  struct multiboot_tag_basic_meminfo *multiboot_meminfo;
  struct multiboot_tag_mmap *multiboot_mmap;
  struct multiboot_tag_framebuffer *multiboot_framebuffer;

  struct multiboot_tag *tag;
  unsigned size = *(unsigned *)addr;
  for (tag = (struct multiboot_tag *)(addr + 8);
       tag->type != MULTIBOOT_TAG_TYPE_END;
       tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
  {
    switch (tag->type)
    {
    case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
      multiboot_meminfo = (struct multiboot_tag_basic_meminfo *)tag;
      break;
    case MULTIBOOT_TAG_TYPE_MMAP:
    {
      multiboot_mmap = (struct multiboot_tag_mmap *)tag;
      break;
    }
    case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
    {
      multiboot_framebuffer = (struct multiboot_tag_framebuffer *)tag;
      break;
    }
    }
  }

  // gdt including kernel, user and tss
  gdt_init();
  install_tss(5, 0x10, 0);

  // register irq and handlers
  idt_init();
  exception_init();

  // timer and keyboard
  pit_init();
  kkybrd_install();
  mouse_init();

  // enable interrupts to start irqs (timer, keyboard)
  enable_interrupts();

  // physical memory and paging
  pmm_init(multiboot_meminfo, multiboot_mmap);
  vmm_init();

  // register system apis
  syscall_init();

  task_init();
  kernel_init();

  console_init(multiboot_framebuffer);
  printf("hello world");

  for (;;)
    ;

  return 0;
}

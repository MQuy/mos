#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "include/multiboot.h"
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
#include "graphics/psf.h"

extern vfs_file_system_type ext2_fs_type;
extern process *current_process;
void kernel_init();

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

  init(boot_info);

  // physical memory and paging
  pmm_init(boot_info);
  vmm_init();

  // register system apis
  syscall_init();

  task_init();
  kernel_init();
  // draw();

  for (;;)
    ;

  return 0;
}

void kernel_init()
{

  pci_scan_buses();

  vfs_init(&ext2_fs_type, "/dev/hda");

  // setup stack and enter user mode
  // setup_and_enter_usermode();
}

char *pram;
uint32_t bpp, pitch;

void init(multiboot_info_t *boot_info)
{
  pram = boot_info->framebuffer_addr;
  bpp = boot_info->framebuffer_bpp;
  pitch = boot_info->framebuffer_pitch;
}

void draw()
{
  char *vram = 0xFC000000;
  uint32_t screen_size = 600 * pitch;
  uint32_t blocks = div_ceil(screen_size, PMM_FRAME_SIZE);
  for (uint32_t i = 0; i < blocks; ++i)
    vmm_map_phyiscal_address(
        vmm_get_directory(),
        vram + i * PMM_FRAME_SIZE,
        pram + i * PMM_FRAME_SIZE,
        I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);

  kstat *stat = malloc(sizeof(kstat));
  sys_stat("/fonts/ter-powerline-v16n.psf", stat);
  unsigned char *buf = pmm_alloc_blocks(3);
  long fd = sys_open("/fonts/ter-powerline-v16n.psf");
  sys_read(fd, buf, stat->size);
  uint32_t length = current_process->files->fd_array[fd]->f_dentry->d_inode->i_size;

  psf_init(buf, length);

  putchar('m', 0, 0, 0xFF0000, 0x0, buf, vram, pitch);
  putchar('i', 1, 0, 0xFF0000, 0x0, buf, vram, pitch);
}
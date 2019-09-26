#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/graphics/DebugDisplay.h>
#include <kernel/fs/vfs.h>
#include "sysapi.h"
#include "tss.h"
#include "task.h"
#include "user.h"

#define USER_STACK_ALLOC_TOP 0xC0000000
#define USER_STACK_BLOCKS 1024

extern void enter_usermode(uint32_t);

uint32_t user_stack_index = 0;
uint32_t create_user_stack(uint32_t blocks)
{
  physical_addr paddr;
  virtual_addr vaddr;

  paddr = pmm_alloc_blocks(blocks);

  if (!paddr)
    return 0;

  user_stack_index += blocks;
  vaddr = USER_STACK_ALLOC_TOP - user_stack_index * PMM_FRAME_SIZE;

  for (int i = 0; i < blocks; ++i)
    vmm_map_phyiscal_address(vmm_get_directory(),
                             vaddr + i * PMM_FRAME_SIZE,
                             paddr + i * PMM_FRAME_SIZE, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);

  return vaddr + blocks * PMM_FRAME_SIZE - 4;
}

void setup_and_enter_usermode()
{
  int esp;
  __asm__ __volatile__("mov %%esp, %0"
                       : "=r"(esp));
  tss_set_stack(0x10, esp);

  task_init();

  enter_usermode(create_user_stack(USER_STACK_BLOCKS));
}

void in_usermode()
{

  long fd = sys_open("/test/aha.txt");
  // char *buf = pmm_alloc_block();
  sys_write(fd, "fuck this file", 14);
  // sys_read(fd, buf, 14);
  // DebugPrintf("content: %s", buf);

  for (;;)
    ;
}

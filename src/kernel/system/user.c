#include "../memory/pmm.h"
#include "../memory/vmm.h"
#include "./sysapi.h"
#include "./tss.h"
#include "user.h"

#define USER_STACK_ALLOC_TOP 0xC0000000
#define USER_STACK_BLOCKS 1024

extern void enter_usermode(uint32_t);
uint32_t create_user_stack();

void setup_and_enter_usermode()
{
  int esp;
  __asm__ __volatile__("mov %%esp, %0"
                       : "=r"(esp));
  tss_set_stack(0x10, esp);

  enter_usermode(create_user_stack());
}

uint32_t create_user_stack()
{
  physical_addr pAddr;
  virtual_addr vAddr;

  pAddr = pmm_alloc_blocks(USER_STACK_BLOCKS);

  if (!pAddr)
    return 0;

  vAddr = USER_STACK_ALLOC_TOP - USER_STACK_BLOCKS * PMM_FRAME_SIZE;

  for (int i = 0; i < USER_STACK_BLOCKS; ++i)
    vmm_map_phyiscal_address(vmm_get_directory(), vAddr + i * PMM_FRAME_SIZE, pAddr + i * PMM_FRAME_SIZE, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);

  return USER_STACK_ALLOC_TOP - 4;
}

void in_usermode()
{
  syscall_printf("\nIn usermode");

  for (;;)
    ;
}
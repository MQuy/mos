#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/graphics/DebugDisplay.h>
#include "sysapi.h"
#include "tss.h"
#include "task.h"
#include "user.h"

#define USER_STACK_ALLOC_TOP 0xC0000000
#define USER_STACK_BLOCKS 1024

extern void enter_usermode(uint32_t);

/*
  NOTE: MQ 2019-05-05
  We created the identity memory map on the first 4MB (paging).
  When creating user stack, we allocate n blocks on physical memory
  if that memory allocation's location exceeds 4MB, page failed happens
*/
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
  run_thread();

  for (;;)
    ;
}

void kthread_1()
{
  while (1)
    for (int i = 0; i < 26; ++i)
    {
      sys_printg(0, 0);
      sys_printc('a' + i);
    }
}

void kthread_2()
{
  while (1)
    for (int i = 0; i < 26; ++i)
    {
      sys_printg(10, 0);
      sys_printc('A' + i);
    }
}

void kthread_3()
{
  while (1)
    for (int i = 0; i < 10; ++i)
    {
      sys_printg(20, 0);
      sys_printc('0' + i);
    }
}

void run_thread()
{
  queue_push(create_thread(kthread_1, (uint32_t)create_user_stack(1)));
  queue_push(create_thread(kthread_2, (uint32_t)create_user_stack(1)));
  queue_push(create_thread(kthread_3, (uint32_t)create_user_stack(1)));

  task_start();
}

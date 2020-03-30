#include <kernel/proc/task.h>
#include "vmm.h"

#define PKMAP_BASE 0xE0000000
#define LAST_PKMAP 1024

extern struct process *current_process;

uint32_t pkmap[LAST_PKMAP];

void pkmap_bitmap_set(uint32_t block)
{
  pkmap[block / 32] |= (1 << (block % 32));
}

void pkmap_bitmap_unset(uint32_t block)
{
  pkmap[block / 32] &= ~(1 << (block % 32));
}

int get_pkmap_free()
{
  for (uint32_t i = 0; i < LAST_PKMAP; ++i)
    if (pkmap[i] != 0xffffffff)
      for (uint32_t j = 0; j < 32; ++j)
      {
        int bit = 1 << j;
        if (!(pkmap[i] & bit))
          return i * 32 + j;
      }

  return -1;
}

void kmap(struct page *p)
{
  uint32_t block = get_pkmap_free();
  uint32_t vaddr = block * PMM_FRAME_SIZE + PKMAP_BASE;

  pkmap_bitmap_set(block);
  vmm_map_address(current_process->pdir, vaddr, p->frame, I86_PTE_PRESENT);
  p->virtual = vaddr;
}

void kunmap(struct page *p)
{
  if (!p->virtual)
    return;

  uint32_t block = (p->virtual - PKMAP_BASE) / PMM_FRAME_SIZE;
  pkmap_bitmap_unset(block);
  vmm_unmap_address(current_process->pdir, p->virtual);
}
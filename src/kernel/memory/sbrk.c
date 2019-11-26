#include "vmm.h"

static uint32_t heap_current = KERNEL_HEAP_BOTTOM;
static uint32_t remaining_from_last_used = 0;

void *sbrk(size_t n)
{
  char *heap_base = heap_current;

  if (n <= remaining_from_last_used)
    remaining_from_last_used -= n;
  else
  {
    uint32_t phyiscal_addr = pmm_alloc_blocks(div_ceil(n - remaining_from_last_used, PMM_FRAME_SIZE));
    uint32_t page_addr = div_ceil(heap_current, PMM_FRAME_SIZE) * PMM_FRAME_SIZE;
    for (; page_addr < heap_current + n; page_addr += PMM_FRAME_SIZE, phyiscal_addr += PMM_FRAME_SIZE)
      vmm_map_phyiscal_address(vmm_get_directory(),
                               page_addr,
                               phyiscal_addr,
                               I86_PTE_PRESENT | I86_PTE_WRITABLE);
    remaining_from_last_used = page_addr - (heap_current + n);
  }

  heap_current += n;
  memset(heap_base, 0, n);
  return heap_base;
}
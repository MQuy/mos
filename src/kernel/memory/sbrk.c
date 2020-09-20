#include <include/errno.h>
#include <kernel/utils/math.h>
#include <kernel/utils/string.h>

#include "vmm.h"

uint32_t kernel_heap_current = KERNEL_HEAP_BOTTOM;
static uint32_t kernel_remaining_from_last_used = 0;

void *sbrk(size_t n)
{
	if (n == 0)
		return (char *)kernel_heap_current;

	char *heap_base = (char *)kernel_heap_current;

	if (n <= kernel_remaining_from_last_used)
		kernel_remaining_from_last_used -= n;
	else
	{
		uint32_t phyiscal_addr = (uint32_t)pmm_alloc_blocks(div_ceil(n - kernel_remaining_from_last_used, PMM_FRAME_SIZE));
		uint32_t page_addr = div_ceil(kernel_heap_current, PMM_FRAME_SIZE) * PMM_FRAME_SIZE;
		for (; page_addr < kernel_heap_current + n; page_addr += PMM_FRAME_SIZE, phyiscal_addr += PMM_FRAME_SIZE)
			vmm_map_address(vmm_get_directory(),
							page_addr,
							phyiscal_addr,
							I86_PTE_PRESENT | I86_PTE_WRITABLE);
		kernel_remaining_from_last_used = page_addr - (kernel_heap_current + n);
	}

	kernel_heap_current += n;
	memset(heap_base, 0, n);
	return heap_base;
}

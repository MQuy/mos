#include <proc/task.h>

#include "vmm.h"

#define PKMAP_BASE 0xE0000000
#define LAST_PKMAP 1024

uint32_t pkmap[LAST_PKMAP];

void pkmap_bitmap_set(uint32_t block)
{
	pkmap[block / 32] |= (1 << (block % 32));
}

void pkmap_bitmap_unset(uint32_t block)
{
	pkmap[block / 32] &= ~(1 << (block % 32));
}

bool pkmap_bitmap_test(uint32_t frame)
{
	return pkmap[frame / 32] & (1 << (frame % 32));
}

int get_pkmap_free()
{
	for (int i = 0; i < LAST_PKMAP; ++i)
		if (pkmap[i] != 0xffffffff)
			for (int j = 0; j < 32; ++j)
			{
				int bit = 1 << j;
				if (!(pkmap[i] & bit))
					return i * 32 + j;
			}

	return -1;
}

int get_pkmaps_free(size_t size)
{
	if (size == 0)
		return -1;

	if (size == 1)
		return get_pkmap_free();

	for (int i = 0; i < LAST_PKMAP; i++)
		if (pkmap[i] != 0xffffffff)
			for (int j = 0; j < 32; j++)
			{  //! test each bit in the dword

				int bit = 1 << j;
				if (!(pkmap[i] & bit))
				{
					int startingBit = i * 32;
					startingBit += j;  //get the free bit in the dword at index i

					uint32_t free = 0;	//loop through each bit to see if its enough space
					for (uint32_t count = 0; count <= size; count++)
					{
						if (!pkmap_bitmap_test(startingBit + count))
							free++;	 // this bit is clear (free frame)

						if (free == size)
							return i * 32 + j;	//free count==size needed; return index
					}
				}
			}

	return -1;
}

void kmap(struct page *p)
{
	uint32_t block = get_pkmap_free();
	uint32_t vaddr = block * PMM_FRAME_SIZE + PKMAP_BASE;

	pkmap_bitmap_set(block);
	vmm_map_address(current_process->pdir, vaddr, p->frame, I86_PTE_PRESENT | I86_PTE_WRITABLE);
	p->virtual = vaddr;
}

void kmaps(struct pages *p)
{
	uint32_t block = get_pkmaps_free(p->number_of_frames);
	uint32_t vaddr = block * PMM_FRAME_SIZE + PKMAP_BASE;

	for (uint32_t i = 0; i < p->number_of_frames; ++i)
	{
		pkmap_bitmap_set(block + i);
		vmm_map_address(current_process->pdir, vaddr + i * PMM_FRAME_SIZE, p->paddr + i * PMM_FRAME_SIZE, I86_PTE_PRESENT | I86_PTE_WRITABLE);
	}
	p->vaddr = vaddr;
}

void kunmap(struct page *p)
{
	if (!p->virtual)
		return;

	uint32_t block = (p->virtual - PKMAP_BASE) / PMM_FRAME_SIZE;
	pkmap_bitmap_unset(block);
	vmm_unmap_address(current_process->pdir, p->virtual);
}

void kunmaps(struct pages *p)
{
	if (!p->vaddr)
		return;

	uint32_t block = (p->vaddr - PKMAP_BASE) / PMM_FRAME_SIZE;
	for (uint32_t i = 0; i < p->number_of_frames; ++i)
	{
		pkmap_bitmap_unset(block + i);
		vmm_unmap_address(current_process->pdir, p->vaddr + i * PMM_FRAME_SIZE);
	}
}

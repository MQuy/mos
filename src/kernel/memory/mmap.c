#include <fs/vfs.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <utils/string.h>

#include "vmm.h"

// TODO: MQ 2020-01-25 Add support for release block when there is no reference to frame block

struct vm_area_struct *get_unmapped_area(uint32_t addr, uint32_t len)
{
	struct mm_struct *mm = current_process->mm;
	struct vm_area_struct *vma = kcalloc(1, sizeof(struct vm_area_struct));
	vma->vm_mm = mm;

	if (!addr || addr < mm->end_brk)
		addr = max(mm->free_area_cache, mm->end_brk);
	addr = PAGE_ALIGN(addr);
	len = PAGE_ALIGN(len);

	uint32_t found_addr = addr;
	if (list_empty(&mm->mmap))
		list_add(&vma->vm_sibling, &mm->mmap);
	else
	{
		struct vm_area_struct *iter = NULL;
		list_for_each_entry(iter, &mm->mmap, vm_sibling)
		{
			struct vm_area_struct *next = list_next_entry(iter, vm_sibling);
			bool is_last_entry = list_is_last(&iter->vm_sibling, &mm->mmap);

			if (addr + len <= iter->vm_start)
			{
				list_add(&iter->vm_sibling, &mm->mmap);
				break;
			}
			else if (addr >= iter->vm_end &&
					 (is_last_entry ||
					  (!is_last_entry && addr + len <= next->vm_start)))
			{
				list_add(&vma->vm_sibling, &iter->vm_sibling);
				break;
			}
			else if (!is_last_entry &&
					 (iter->vm_end <= addr && addr < next->vm_start) && next->vm_start - iter->vm_end >= len)
			{
				list_add(&vma->vm_sibling, &iter->vm_sibling);
				found_addr = next->vm_start - len;
				break;
			}
		}
	}

	if (found_addr)
	{
		vma->vm_start = found_addr;
		vma->vm_end = found_addr + len;
		mm->free_area_cache = vma->vm_end;
	}

	return vma;
}

struct vm_area_struct *find_vma(struct mm_struct *mm, uint32_t addr)
{
	struct vm_area_struct *iter = NULL;
	list_for_each_entry(iter, &mm->mmap, vm_sibling)
	{
		if (iter->vm_start <= addr && addr < iter->vm_end)
			return iter;
	}

	return NULL;
}

// NOTE: MQ 2020-01-25 We only support unmap in one area
int do_munmap(struct mm_struct *mm, uint32_t addr, size_t len)
{
	struct vm_area_struct *vma = find_vma(mm, addr);
	if (!vma || vma->vm_start < addr)
		return 0;

	len = PAGE_ALIGN(len);
	if (vma->vm_end - vma->vm_start >= len)
		vma->vm_end = addr;
	else
		list_del(&vma->vm_sibling);

	return 0;
}

int32_t do_mmap(uint32_t addr,
				size_t len, uint32_t prot,
				uint32_t flag, int32_t fd)
{
	struct vfs_file *file = fd >= 0 ? current_process->files->fd[fd] : NULL;
	struct vm_area_struct *vma = get_unmapped_area(addr, len);

	if (file)
	{
		file->f_op->mmap(file, vma);
		vma->vm_file = file;
	}
	else
		for (uint32_t vaddr = vma->vm_start; vaddr < vma->vm_end; vaddr += PMM_FRAME_SIZE)
		{
			uint32_t paddr = (uint32_t)pmm_alloc_block();
			vmm_map_address(current_process->pdir, vaddr, paddr, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
		}

	return vma->vm_start;
}

int expand_area(struct vm_area_struct *vma, uint32_t address)
{
	address = PAGE_ALIGN(address);
	if (address <= vma->vm_end)
		return 0;

	if (list_is_last(&vma->vm_sibling, &current_process->mm->mmap))
		vma->vm_end = address;
	else
	{
		struct vm_area_struct *next = list_next_entry(vma, vm_sibling);
		if (address <= next->vm_start)
			vma->vm_end = address;
		else
		{
			list_del(&vma->vm_sibling);
			struct vm_area_struct *vma_expand = get_unmapped_area(0, address - vma->vm_start);
			memcpy(vma, vma_expand, sizeof(struct vm_area_struct));
			kfree(vma_expand);
		}
	}
	return 0;
}

void shift_area(struct vm_area_struct *vma, struct vm_area_struct *new_vma)
{
	if (vma->vm_start == new_vma->vm_start)
	{
		if (new_vma->vm_end > vma->vm_end)
		{
			uint32_t nframes = (new_vma->vm_end - vma->vm_end) / PMM_FRAME_SIZE;
			uint32_t paddr = (uint32_t)pmm_alloc_blocks(nframes);
			for (uint32_t vaddr = vma->vm_end; vaddr < new_vma->vm_end; vaddr += PMM_FRAME_SIZE, paddr += PMM_FRAME_SIZE)
				vmm_map_address(current_process->pdir, vaddr, paddr, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
		}
		else if (new_vma->vm_end < vma->vm_end)
			for (uint32_t addr = new_vma->vm_end; addr < vma->vm_end; addr += PMM_FRAME_SIZE)
				vmm_unmap_address(current_process->pdir, addr);
	}
	else
	{
		uint32_t old_length = vma->vm_end - vma->vm_start;
		uint32_t new_length = new_vma->vm_end - new_vma->vm_start;
		for (uint32_t vaddr = 0; vaddr < new_length; vaddr += PMM_FRAME_SIZE)
		{
			uint32_t paddr = vaddr < old_length ? vmm_get_physical_address(vma->vm_start + vaddr, false) : (uint32_t)pmm_alloc_block();
			vmm_map_address(current_process->pdir,
							new_vma->vm_start + vaddr,
							paddr,
							I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
		};
	}
}

// FIXME: MQ 2019-01-16 Currently, we assume that start_brk is not changed
uint32_t do_brk(uint32_t addr, size_t len)
{
	struct mm_struct *mm = current_process->mm;
	struct vm_area_struct *vma = find_vma(mm, addr);
	uint32_t new_brk = PAGE_ALIGN(addr + len);
	mm->brk = new_brk;

	if (!vma || vma->vm_end >= new_brk)
		return 0;

	struct vm_area_struct *new_vma = kcalloc(1, sizeof(struct vm_area_struct));
	memcpy(new_vma, vma, sizeof(struct vm_area_struct));
	if (new_brk > mm->brk)
		expand_area(new_vma, new_brk);
	else
		new_vma->vm_end = new_brk;

	if (vma->vm_file)
		vma->vm_file->f_op->mmap(vma->vm_file, new_vma);
	else
		shift_area(vma, new_vma);
	memcpy(vma, new_vma, sizeof(struct vm_area_struct));

	return 0;
}

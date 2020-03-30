#ifndef MEMORY_VMM_H
#define MEMORY_VMM_H

#include <stdint.h>
#include <include/list.h>
#include "pmm.h"
#include "kernel_info.h"

#define KERNEL_HEAP_TOP 0xF0000000
#define KERNEL_HEAP_BOTTOM 0xD0000000
#define USER_HEAP_TOP 0x40000000

struct vm_area_struct;
struct mm_struct;

//! i86 architecture defines this format so be careful if you modify it
enum PAGE_PTE_FLAGS
{

  I86_PTE_PRESENT = 1,          //0000000000000000000000000000001
  I86_PTE_WRITABLE = 2,         //0000000000000000000000000000010
  I86_PTE_USER = 4,             //0000000000000000000000000000100
  I86_PTE_WRITETHOUGH = 8,      //0000000000000000000000000001000
  I86_PTE_NOT_CACHEABLE = 0x10, //0000000000000000000000000010000
  I86_PTE_ACCESSED = 0x20,      //0000000000000000000000000100000
  I86_PTE_DIRTY = 0x40,         //0000000000000000000000001000000
  I86_PTE_PAT = 0x80,           //0000000000000000000000010000000
  I86_PTE_CPU_GLOBAL = 0x100,   //0000000000000000000000100000000
  I86_PTE_LV4_GLOBAL = 0x200,   //0000000000000000000001000000000
  I86_PTE_FRAME = 0x7FFFF000    //1111111111111111111000000000000
};

typedef uint32_t pt_entry;

//! this format is defined by the i86 architecture--be careful if you modify it
enum PAGE_PDE_FLAGS
{

  I86_PDE_PRESENT = 1,        //0000000000000000000000000000001
  I86_PDE_WRITABLE = 2,       //0000000000000000000000000000010
  I86_PDE_USER = 4,           //0000000000000000000000000000100
  I86_PDE_PWT = 8,            //0000000000000000000000000001000
  I86_PDE_PCD = 0x10,         //0000000000000000000000000010000
  I86_PDE_ACCESSED = 0x20,    //0000000000000000000000000100000
  I86_PDE_DIRTY = 0x40,       //0000000000000000000000001000000
  I86_PDE_4MB = 0x80,         //0000000000000000000000010000000
  I86_PDE_CPU_GLOBAL = 0x100, //0000000000000000000000100000000
  I86_PDE_LV4_GLOBAL = 0x200, //0000000000000000000001000000000
  I86_PDE_FRAME = 0x7FFFF000  //1111111111111111111000000000000
};

typedef uint32_t pd_entry;

//! virtual address
typedef uint32_t virtual_addr;

//! i86 architecture defines 1024 entries per table--do not change
#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR 1024

struct page
{
  uint32_t frame;
  struct list_head sibling;
  uint32_t virtual;
};

struct ptable
{
  pt_entry m_entries[PAGES_PER_TABLE];
};

struct pdirectory
{
  pd_entry m_entries[PAGES_PER_DIR];
};

void vmm_init();
struct pdirectory *vmm_get_directory();
void vmm_map_address(struct pdirectory *dir, uint32_t virt, uint32_t phys, uint32_t flags);
void vmm_unmap_address(struct pdirectory *va_dir, uint32_t virt);
void *create_kernel_stack(int32_t blocks);
struct pdirectory *vmm_create_address_space(struct pdirectory *dir);
physical_addr vmm_get_physical_address(virtual_addr vaddr);
struct pdirectory *vmm_fork(struct pdirectory *va_dir);

// malloc.c
void *sbrk(size_t n);
void *kmalloc(size_t n);
void *kcalloc(size_t n, size_t size);
void *krealloc(void *ptr, size_t size);
void kfree(void *ptr);
void *kalign_heap(size_t size);

// mmap.c
struct vm_area_struct *get_unmapped_area(uint32_t addr, uint32_t len);
int expand_stack(struct vm_area_struct *vma, unsigned long address);
int32_t do_mmap(uint32_t addr,
                size_t len, uint32_t prot,
                uint32_t flag, int32_t fd);
int do_munmap(struct mm_struct *mm, uint32_t addr, size_t len);
uint32_t do_brk(uint32_t addr, size_t len);

// highmem.c
void kmap(struct page *p);
void kunmap(struct page *p);

#endif
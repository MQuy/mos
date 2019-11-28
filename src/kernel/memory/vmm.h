#ifndef MEMORY_VMM_H
#define MEMORY_VMM_H

#include <stdint.h>
#include "pmm.h"
#include "kernel_info.h"

#define KERNEL_HEAP_BOTTOM 0xD0000000
#define KERNEL_HEAP_TOP 0xF0000000
#define USER_STACK_BOTTOM 0xAFF00000
#define USER_STACK_TOP 0xB0000000

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

typedef struct ptable
{
  pt_entry m_entries[PAGES_PER_TABLE];
} ptable;

typedef struct pdirectory
{
  pd_entry m_entries[PAGES_PER_DIR];
} pdirectory;

void vmm_init();
pdirectory *vmm_get_directory();
void vmm_map_phyiscal_address(pdirectory *dir, uint32_t virt, uint32_t phys, uint32_t flags);
void *create_kernel_stack(int32_t blocks);
pdirectory *create_address_space(pdirectory *dir);
physical_addr vmm_get_physical_address(virtual_addr vaddr);

extern void *sbrk(size_t n);

#endif
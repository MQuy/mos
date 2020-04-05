#include "vmm.h"

#define PAGE_DIRECTORY_BASE 0xFFFFF000
#define PAGE_TABLE_BASE 0xFFC00000

#define get_page_directory_index(x) (((x) >> 22) & 0x3ff)
#define get_page_table_entry_index(x) (((x) >> 12) & 0x3ff)
#define get_aligned_address(x) (x & ~0xfff)
#define is_page_enabled(x) (x & 0x1)

void vmm_init_and_map(struct pdirectory *, uint32_t, uint32_t);
void vmm_alloc_ptable(struct pdirectory *va_dir, uint32_t index);
void pt_entry_add_attrib(pt_entry *, uint32_t);
void pt_entry_set_frame(pt_entry *, uint32_t);
void pd_entry_add_attrib(pd_entry *, uint32_t);
void pd_entry_set_frame(pd_entry *, uint32_t);
void vmm_create_page_table(struct pdirectory *dir, uint32_t virt, uint32_t flags);
void vmm_paging(struct pdirectory *, uint32_t);

static struct pdirectory *_current_dir;

void vmm_flush_tlb_entry(uint32_t addr)
{

  __asm__ __volatile__("invlpg (%0)" ::"r"(addr)
                       : "memory");
}

/*
  Memory layout of our address space
  +-------------------------+ 0xFFFFFFFF
  | Page table mapping      |
  |_________________________| 0xFFC00000
  |                         |
  |-------------------------| 0xF0000000
  |                         |
  | Device drivers          |
  |                         |
  |-------------------------| 0xE8000000
  | VMALLOC                 |
  |-------------------------| 0xE0000000
  |                         |
  |                         |
  | Kernel heap             |
  |                         |
  |_________________________| 0xC8000000
  |                         | 
  | Kernel itself           |
  |_________________________| 0xC0000000
  |                         |
  | Page for page faults    |
  |_________________________| 0xBFFFF000
  |                         |
  |                         |
  |                         |
  |                         |
  |_________________________| 0x40000000
  |                         |
  | User heap               |
  |_________________________| 0x
  |                         |
  | Elf                     |
  |_________________________| 0x100000
  |                         |
  | Reversed for devices    |
  +_________________________+ 0x00000000
*/

void vmm_init()
{
  // initialize page table directory

  uint32_t pa_dir = (uint32_t)pmm_alloc_block();
  struct pdirectory *va_dir = (struct pdirectory *)(pa_dir + KERNEL_HIGHER_HALF);
  memset(va_dir, 0, sizeof(struct pdirectory));

  vmm_init_and_map(va_dir, 0xC0000000, 0x00000000);

  // NOTE: MQ 2019-11-21 Preallocate ptable for higher half kernel
  for (uint32_t i = 769; i < 1024; ++i)
    vmm_alloc_ptable(va_dir, i);

  // NOTE: MQ 2019-05-08 Using the recursive page directory trick when paging (map last entry to directory)
  va_dir->m_entries[1023] = (pa_dir & 0xFFFFF000) | I86_PTE_PRESENT | I86_PTE_WRITABLE;

  vmm_paging(va_dir, pa_dir);
}

void vmm_alloc_ptable(struct pdirectory *va_dir, uint32_t index)
{
  if (is_page_enabled(va_dir->m_entries[index]))
    return;

  uint32_t pa_table = (uint32_t)pmm_alloc_block();
  va_dir->m_entries[index] = pa_table | I86_PDE_PRESENT | I86_PDE_WRITABLE;
}

void vmm_init_and_map(struct pdirectory *va_dir, uint32_t vaddr, uint32_t paddr)
{
  uint32_t pa_table = (uint32_t)pmm_alloc_block();
  struct ptable *va_table = (struct ptable *)(pa_table + KERNEL_HIGHER_HALF);
  memset(va_table, 0, sizeof(struct ptable));

  uint32_t ivirtual = vaddr;
  uint32_t iframe = paddr;

  for (int i = 0; i < 1024; ++i, ivirtual += PMM_FRAME_SIZE, iframe += PMM_FRAME_SIZE)
  {
    pt_entry *entry = &va_table->m_entries[get_page_table_entry_index(ivirtual)];
    *entry = iframe | I86_PTE_PRESENT | I86_PTE_WRITABLE;
    pmm_mark_used_addr(iframe);
  }

  pd_entry *entry = &va_dir->m_entries[get_page_directory_index(vaddr)];
  *entry = pa_table | I86_PDE_PRESENT | I86_PDE_WRITABLE;
}

void vmm_paging(struct pdirectory *va_dir, uint32_t pa_dir)
{
  _current_dir = va_dir;

  __asm__ __volatile__("mov %0, %%cr3           \n"
                       "mov %%cr4, %%ecx        \n"
                       "and $~0x00000010, %%ecx \n"
                       "mov %%ecx, %%cr4        \n"
                       "mov %%cr0, %%ecx        \n"
                       "or $0x80000000, %%ecx   \n"
                       "mov %%ecx, %%cr0        \n" ::"r"(pa_dir));
}

void pt_entry_add_attrib(pt_entry *e, uint32_t attr)
{
  *e |= attr;
}

void pt_entry_set_frame(pt_entry *e, uint32_t addr)
{
  *e = (*e & ~I86_PTE_FRAME) | addr;
}

void pd_entry_add_attrib(pd_entry *e, uint32_t attr)
{
  *e |= attr;
}

void pd_entry_set_frame(pd_entry *e, uint32_t addr)
{
  *e = (*e & ~I86_PDE_FRAME) | addr;
}

uint32_t vmm_get_physical_address(uint32_t vaddr, bool is_page)
{
  uint32_t *table = (uint32_t *)((char *)PAGE_TABLE_BASE + get_page_directory_index(vaddr) * PMM_FRAME_SIZE);
  uint32_t tindex = get_page_table_entry_index(vaddr);
  uint32_t paddr = table[tindex];
  if (is_page)
    return paddr;
  else
    return (paddr & ~0xfff) | (vaddr & 0xfff);
}

struct pdirectory *vmm_create_address_space(struct pdirectory *current)
{
  char *aligned_object = kalign_heap(PMM_FRAME_SIZE);
  // NOTE: MQ 2019-11-24 page directory, page table have to be aligned by 4096
  struct pdirectory *va_dir = kcalloc(1, sizeof(struct pdirectory));
  if (aligned_object)
    kfree(aligned_object);

  if (!va_dir)
    return NULL;

  for (uint32_t i = 768; i < 1023; ++i)
    va_dir->m_entries[i] = vmm_get_physical_address(PAGE_TABLE_BASE + i * PMM_FRAME_SIZE, true);

  // NOTE: MQ 2019-11-26 Recursive paging for new page directory
  va_dir->m_entries[1023] = vmm_get_physical_address((uint32_t)va_dir, true);

  return va_dir;
}

struct pdirectory *vmm_get_directory()
{
  return _current_dir;
}

/*
  NOTE: MQ 2019-05-08
  cr3 -> 0xsomewhere (physical address): | 0 | 1 | ... | 1023 | (1023th pd's value is 0xsomewhere)
  When translating a virtual address vAddr:
    + de is ath page directory index (vAddr >> 22)
    + te is bth page table index (vAddr >> 12 & 0x1111111111)
  If pd[de] is not present, getting 4KB free from pmm_alloc_block() and assigning it at page directory entry

  mmus' formula: pd = cr3(); pt = *(pd+4*PDX); page = *(pt+4*PTX)
  The issue is we cannot directly access *(pd+4*PDX) and access/modify a page table entry because everything(kernel) is now a virtual address
  To overcome the egg-chicken's issue, we set 1023pde=cr3 

  For example:
  0xFFC00000 + de * 0x1000 is mapped to pd[de] (physical address). As you know virtual <-> physical address is 4KB aligned
  0xFFC00000 + de * 0x1000 + te * 0x4 is mapped to pd[de] + te * 0x4 (this is what mmu will us to translate vAddr)
  0xFFC00000 + de * 0x1000 + te * 0x4 = xxx <-> *(pt+4*ptx) = xxx
*/
void vmm_map_address(struct pdirectory *va_dir, uint32_t virt, uint32_t phys, uint32_t flags)
{
  if (!is_page_enabled(va_dir->m_entries[get_page_directory_index(virt)]))
    vmm_create_page_table(va_dir, virt, flags);

  uint32_t *table = (uint32_t *)((char *)PAGE_TABLE_BASE + get_page_directory_index(virt) * PMM_FRAME_SIZE);
  uint32_t tindex = get_page_table_entry_index(virt);

  table[tindex] = phys | flags;
}

void vmm_create_page_table(struct pdirectory *va_dir, uint32_t virt, uint32_t flags)
{
  if (is_page_enabled(va_dir->m_entries[get_page_directory_index(virt)]))
    return;

  uint32_t pa_table = (uint32_t)pmm_alloc_block();

  va_dir->m_entries[get_page_directory_index(virt)] = pa_table | flags;
  vmm_flush_tlb_entry(virt);

  memset((char *)PAGE_TABLE_BASE + get_page_directory_index(virt) * PMM_FRAME_SIZE, 0, sizeof(struct ptable));
}

void vmm_unmap_address(struct pdirectory *va_dir, uint32_t virt)
{
  if (!is_page_enabled(va_dir->m_entries[get_page_directory_index(virt)]))
    return;

  struct ptable *pt = (struct ptable *)(PAGE_TABLE_BASE + get_page_directory_index(virt) * PMM_FRAME_SIZE);
  uint32_t pte = get_page_table_entry_index(virt);

  if (!is_page_enabled(pt->m_entries[pte]))
    return;

  pt->m_entries[pte] = 0;
  vmm_flush_tlb_entry(virt);
}

struct pdirectory *vmm_fork(struct pdirectory *va_dir)
{
  struct pdirectory *forked_dir = vmm_create_address_space(va_dir);
  char *aligned_object = kalign_heap(PMM_FRAME_SIZE);
  uint32_t heap_current = (uint32_t)sbrk(0);

  // NOTE: MQ 2019-12-15 Any heap changes via malloc is forbidden
  for (uint32_t ipd = 0; ipd < 768; ++ipd)
    if (is_page_enabled(va_dir->m_entries[ipd]))
    {
      struct ptable *forked_pt = (struct ptable *)heap_current;
      uint32_t forked_pt_paddr = (uint32_t)pmm_alloc_block();
      vmm_map_address(va_dir, forked_pt, forked_pt_paddr, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
      memset(forked_pt, 0, sizeof(struct ptable));

      heap_current += sizeof(struct ptable);
      struct ptable *pt = (struct ptable *)(PAGE_TABLE_BASE + ipd * PMM_FRAME_SIZE);
      for (uint32_t ipt = 0; ipt < PAGES_PER_TABLE; ++ipt)
      {
        if (is_page_enabled(pt->m_entries[ipt]))
        {
          char *pte = (char *)heap_current;
          char *forked_pte = pte + PMM_FRAME_SIZE;
          heap_current = (uint32_t)(forked_pte + PMM_FRAME_SIZE);
          uint32_t forked_pte_paddr = (uint32_t)pmm_alloc_block();

          vmm_map_address(va_dir, (uint32_t)pte, pt->m_entries[ipt], I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
          vmm_map_address(va_dir, (uint32_t)forked_pte, forked_pte_paddr, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);

          memcpy(forked_pte, pte, PMM_FRAME_SIZE);

          vmm_unmap_address(va_dir, (uint32_t)pte);
          vmm_unmap_address(va_dir, (uint32_t)forked_pte);

          forked_pt->m_entries[ipt] = forked_pte_paddr | I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER;
        }
      }
      vmm_unmap_address(va_dir, forked_pt);
      forked_dir->m_entries[ipd] = forked_pt_paddr | I86_PDE_PRESENT | I86_PDE_WRITABLE | I86_PDE_USER;
    }

  if (aligned_object)
    kfree(aligned_object);
  return forked_dir;
}
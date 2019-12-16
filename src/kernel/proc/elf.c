#include <include/errno.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/malloc.h>
#include "elf.h"

#define NO_ERROR 0
#define ERR_NOT_ELF_FILE 1
#define ERR_NOT_SUPPORTED_PLATFORM 2
#define ERR_NOT_SUPPORTED_ENCODING 3
#define ERR_WRONG_VERSION 4
#define ERR_NOT_SUPPORTED_TYPE 5

int elf_verify(Elf32_Ehdr *elf_header)
{
  if (!(elf_header->e_ident[EI_MAG0] == ELFMAG0 &&
        elf_header->e_ident[EI_MAG1] == ELFMAG1 &&
        elf_header->e_ident[EI_MAG2] == ELFMAG2 &&
        elf_header->e_ident[EI_MAG3] == ELFMAG3))
    return -ERR_NOT_ELF_FILE;

  if (elf_header->e_ident[EI_CLASS] != ELFCLASS32)
    return -ERR_NOT_SUPPORTED_PLATFORM;

  if (elf_header->e_ident[EI_DATA] != ELFDATA2LSB)
    return -ERR_NOT_SUPPORTED_ENCODING;

  if (elf_header->e_ident[EI_VERSION] != EV_CURRENT)
    return -ERR_WRONG_VERSION;

  if (elf_header->e_machine != EM_386)
    return -ERR_NOT_SUPPORTED_PLATFORM;

  if (elf_header->e_type != ET_EXEC)
    return -ERR_NOT_SUPPORTED_TYPE;

  return NO_ERROR;
}

void elf_paging(pdirectory *pdir, virtual_addr vaddr, uint32_t size, uint32_t flags)
{
  virtual_addr aligned_vaddr = (vaddr / PMM_FRAME_SIZE) * PMM_FRAME_SIZE;
  uint32_t padding = vaddr - aligned_vaddr;
  uint32_t blocks = div_ceil(size + padding, PMM_FRAME_SIZE);
  physical_addr paddr = pmm_alloc_blocks(blocks);

  for (uint32_t i = 0; i < blocks; ++i)
    vmm_map_address(pdir, vaddr + i * PMM_FRAME_SIZE, paddr + i * PMM_FRAME_SIZE, flags);
}

/*
* 	+---------------+ 	/
* 	| stack pointer | grows toward lower addresses
* 	+---------------+ ||
* 	|...............| \/
* 	|...............|
* 	|...............|
* 	|...............| /\
* 	+---------------+ ||
* 	|  brk (heap)   | grows toward higher addresses
* 	+---------------+
* 	| .bss section  |
* 	+---------------+
* 	| .data section |
* 	+---------------+
* 	| .text section |
* 	+---------------+
*/
Elf32_Layout *elf_load(char *buf, pdirectory *pdir)
{
  Elf32_Ehdr *elf_header = (Elf32_Ehdr *)buf;

  if (elf_verify(elf_header) != NO_ERROR || elf_header->e_phoff == NULL)
    return NULL;

  Elf32_Layout *layout = malloc(sizeof(Elf32_Layout));
  layout->entry = elf_header->e_entry;
  for (Elf32_Phdr *ph = buf + elf_header->e_phoff;
       ph && ph < (buf + elf_header->e_phoff + elf_header->e_phentsize * elf_header->e_phnum);
       ++ph)
  {
    if (ph->p_type != PT_LOAD)
      continue;

    // text segment
    if ((ph->p_flags & PF_X) != 0 && (ph->p_flags & PF_R) != 0)
    {
      elf_paging(pdir, ph->p_vaddr, ph->p_memsz, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
    }
    // data segment
    else if ((ph->p_flags & PF_W) != 0 && (ph->p_flags & PF_R) != 0)
    {
      elf_paging(pdir, ph->p_vaddr, ph->p_memsz, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
    }

    // NOTE: MQ 2019-11-26 According to elf's spec, p_memsz may be larger than p_filesz due to bss section
    memset(ph->p_vaddr, 0, ph->p_memsz);
    memcpy(ph->p_vaddr, buf + ph->p_offset, ph->p_filesz);
  }

  layout->stack = USER_STACK_TOP;
  for (virtual_addr vaddr = USER_STACK_BOTTOM; vaddr < USER_STACK_TOP; vaddr += PMM_FRAME_SIZE)
  {
    physical_addr paddr = pmm_alloc_block();
    vmm_map_address(pdir, vaddr, paddr, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
  }

  return layout;
}

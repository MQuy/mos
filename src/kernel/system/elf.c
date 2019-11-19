#include <kernel/include/errno.h>
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
    return ERR_NOT_ELF_FILE;

  if (elf_header->e_ident[EI_CLASS] != ELFCLASS32)
    return ERR_NOT_SUPPORTED_PLATFORM;

  if (elf_header->e_ident[EI_DATA] != ELFDATA2LSB)
    return ERR_NOT_SUPPORTED_ENCODING;

  if (elf_header->e_ident[EI_VERSION] != EV_CURRENT)
    return ERR_WRONG_VERSION;

  if (elf_header->e_machine != EM_386)
    return ERR_NOT_SUPPORTED_PLATFORM;

  if (elf_header->e_type != ET_EXEC)
    return ERR_NOT_SUPPORTED_TYPE;

  return NO_ERROR;
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
int elf_load(char *buf, pdirectory *pdir)
{
  Elf32_Ehdr *elf_header = (Elf32_Ehdr *)buf;

  if (elf_verify(elf_header) != NO_ERROR || elf_header->e_phoff == NULL)
    return -EINVAL;

  for (Elf32_Phdr *ph = elf_header->e_phoff;
       ph && ph < (elf_header->e_phoff + elf_header->e_phentsize * elf_header->e_phnum);
       ++ph)
  {
    if (ph->p_type == PT_LOAD)
    {
    }
  }
}

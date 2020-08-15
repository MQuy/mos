#include "elf.h"

#include <include/errno.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include <kernel/utils/string.h>

#define NO_ERROR 0
#define ERR_NOT_ELF_FILE 1
#define ERR_NOT_SUPPORTED_PLATFORM 2
#define ERR_NOT_SUPPORTED_ENCODING 3
#define ERR_WRONG_VERSION 4
#define ERR_NOT_SUPPORTED_TYPE 5

int elf_verify(struct Elf32_Ehdr *elf_header)
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
struct Elf32_Layout *elf_load(char *buf)
{
	struct Elf32_Ehdr *elf_header = (struct Elf32_Ehdr *)buf;

	if (elf_verify(elf_header) != NO_ERROR || elf_header->e_phoff == 0)
		return NULL;

	struct mm_struct *mm = current_process->mm;
	struct Elf32_Layout *layout = kcalloc(1, sizeof(struct Elf32_Layout));
	layout->entry = elf_header->e_entry;
	for (struct Elf32_Phdr *ph = (struct Elf32_Phdr *)(buf + elf_header->e_phoff);
		 ph && (char *)ph < (buf + elf_header->e_phoff + elf_header->e_phentsize * elf_header->e_phnum);
		 ++ph)
	{
		if (ph->p_type != PT_LOAD)
			continue;

		// text segment
		if ((ph->p_flags & PF_X) != 0 && (ph->p_flags & PF_R) != 0)
		{
			do_mmap(ph->p_vaddr, ph->p_memsz, 0, 0, -1);
			mm->start_code = ph->p_vaddr;
			mm->end_code = ph->p_vaddr + ph->p_memsz;
		}
		// data segment
		else if ((ph->p_flags & PF_W) != 0 && (ph->p_flags & PF_R) != 0)
		{
			do_mmap(ph->p_vaddr, ph->p_memsz, 0, 0, -1);
			mm->start_data = ph->p_vaddr;
			mm->end_data = ph->p_memsz;
		}
		// eh frame
		else
			do_mmap(ph->p_vaddr, ph->p_memsz, 0, 0, -1);

		// NOTE: MQ 2019-11-26 According to elf's spec, p_memsz may be larger than p_filesz due to bss section
		memset((char *)ph->p_vaddr, 0, ph->p_memsz);
		memcpy((char *)ph->p_vaddr, buf + ph->p_offset, ph->p_filesz);
	}

	uint32_t heap_start = do_mmap(0, UHEAP_SIZE, 0, 0, -1);
	mm->start_brk = heap_start;
	mm->brk = heap_start;
	mm->end_brk = USER_HEAP_TOP;

	uint32_t stack_start = do_mmap(0, STACK_SIZE, 0, 0, -1);
	layout->stack = stack_start + STACK_SIZE;

	return layout;
}

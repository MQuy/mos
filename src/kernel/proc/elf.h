#ifndef PROC_ELF_H
#define PROC_ELF_H

#include <stdint.h>
#include <kernel/fs/vfs.h>
#include <kernel/memory/vmm.h>

typedef uint16_t Elf32_Half; // Unsigned half int
typedef uint32_t Elf32_Off;  // Unsigned offset
typedef uint32_t Elf32_Addr; // Unsigned address
typedef uint32_t Elf32_Word; // Unsigned int
typedef int32_t Elf32_Sword; // Signed int

// e_ident
#define EI_MAG0 0       // 0x7F
#define EI_MAG1 1       // 'E'
#define EI_MAG2 2       // 'L'
#define EI_MAG3 3       // 'F'
#define EI_CLASS 4      // Architecture (32/64)
#define EI_DATA 5       // Byte Order
#define EI_VERSION 6    // ELF Version
#define EI_OSABI 7      // OS Specific
#define EI_ABIVERSION 8 // OS Specific
#define EI_PAD = 9      // Padding
#define EI_NIDENT 16

// e_ident[EI_MAGIC]
#define ELFMAG0 0x7F // e_ident[EI_MAG0]
#define ELFMAG1 'E'  // e_ident[EI_MAG1]
#define ELFMAG2 'L'  // e_ident[EI_MAG2]
#define ELFMAG3 'F'  // e_ident[EI_MAG3]

// e_ident[EI_CLASS]
#define ELFCLASSNONE 0 // Invalid class
#define ELFCLASS32 1   // 32-bit objects
#define ELFCLASS64 2   // 64-bit objects

// e_ident[EI_DATA]
#define ELFDATANONE 0 // Invalid data encoding
#define ELFDATA2LSB 1 // Little Endian
#define ELFDATA2MSB 2 // Big Endian

// e_ident[EI_VERSION]
#define EV_NONE 0    // Invalid version
#define EV_CURRENT 1 // Current version

// e_type
#define ET_NONE 0 // Unkown Type
#define ET_REL 1  // Relocatable File
#define ET_EXEC 2 // Executable File
#define ET_DYN 3  // Share object file
#define ET_CORE 4 // Core file

// e_machine
#define EM_NONE 0  // No machine
#define EM_M32 1   // AT&T WE 32100
#define EM_SPARC 2 // Sun Microsystems SPARC
#define EM_386 3   // Intel 80386
#define EM_68K 4   // Motorola 68000
#define EM_88K 5   // Motorola 88000
#define EM_486 6   /* Perhaps disused */
#define EM_860 7   // Intel 80860
#define EM_MIPS 8  // MIPS RS3000 (big-endian only)

typedef struct
{
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half e_type;
  Elf32_Half e_machine;
  Elf32_Word e_version;
  Elf32_Addr e_entry; /* Entry point */
  Elf32_Off e_phoff;
  Elf32_Off e_shoff;
  Elf32_Word e_flags;
  Elf32_Half e_ehsize;
  Elf32_Half e_phentsize;
  Elf32_Half e_phnum;
  Elf32_Half e_shentsize;
  Elf32_Half e_shnum;
  Elf32_Half e_shstrndx;
} Elf32_Ehdr;

// sh_type
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_NUM 12
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

// sh_flags
#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC 0xf0000000

// special section indexes
#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff

typedef struct
{
  Elf32_Word sh_name;
  Elf32_Word sh_type;
  Elf32_Word sh_flags;
  Elf32_Addr sh_addr;
  Elf32_Off sh_offset;
  Elf32_Word sh_size;
  Elf32_Word sh_link;
  Elf32_Word sh_info;
  Elf32_Word sh_addralign;
  Elf32_Word sh_entsize;
} Elf32_Shdr;

#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_TLS 7           /* Thread local storage segment */
#define PT_LOOS 0x60000000 /* OS-specific */
#define PT_HIOS 0x6fffffff /* OS-specific */
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff
#define PT_GNU_EH_FRAME 0x6474e550

#define PT_GNU_STACK (PT_LOOS + 0x474e551)

typedef struct
{
  Elf32_Word p_type;
  Elf32_Off p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
} Elf32_Phdr;

typedef struct Elf32_Layout
{
  uint32_t stack;
  uint32_t entry;
} Elf32_Layout;

Elf32_Layout *elf_load(char *);

#endif
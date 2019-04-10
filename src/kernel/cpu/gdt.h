#ifndef CPU_GDT_H
#define CPU_GDT_H

#include <stdint.h>

//! maximum amount of descriptors allowed
#define MAX_DESCRIPTORS 3

/***	 gdt descriptor access bit flags.	***/

//! set access bit
#define I86_GDT_DESC_ACCESS 0x0001 //00000001

//! descriptor is readable and writable. default: read only
#define I86_GDT_DESC_READWRITE 0x0002 //00000010

//! set expansion direction bit
#define I86_GDT_DESC_EXPANSION 0x0004 //00000100

//! executable code segment. Default: data segment
#define I86_GDT_DESC_EXEC_CODE 0x0008 //00001000

//! set code or data descriptor. defult: system defined descriptor
#define I86_GDT_DESC_CODEDATA 0x0010 //00010000

//! set dpl bits
#define I86_GDT_DESC_DPL 0x0060 //01100000

//! set "in memory" bit
#define I86_GDT_DESC_MEMORY 0x0080 //10000000

/**	gdt descriptor grandularity bit flags	***/

//! masks out limitHi (High 4 bits of limit)
#define I86_GDT_GRAND_LIMITHI_MASK 0x0f //00001111

//! set os defined bit
#define I86_GDT_GRAND_OS 0x10 //00010000

//! set if 32bit. default: 16 bit
#define I86_GDT_GRAND_32BIT 0x40 //01000000

//! 4k grandularity. default: none
#define I86_GDT_GRAND_4K 0x80 //10000000

typedef struct gdt_descriptor
{
  uint16_t limit;

  uint16_t baseLo;
  uint8_t baseMid;

  uint8_t flags;
  uint8_t grand;

  uint8_t baseHi;
} __attribute__((packed)) gdt_descriptor;

typedef struct gdtr
{
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) gdtr;

void gdt_init();

#endif
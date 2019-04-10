#ifndef CPU_IDT_H
#define CPU_IDT_H

#include <stdint.h>
#include "../include/string.h"

//! i86 defines 256 possible interrupt handlers (0-255)
#define I86_MAX_INTERRUPTS 256

//! must be in the format 0D110, where D is descriptor type
#define I86_IDT_DESC_BIT16 0x06   //00000110
#define I86_IDT_DESC_BIT32 0x0E   //00001110
#define I86_IDT_DESC_RING1 0x40   //01000000
#define I86_IDT_DESC_RING2 0x20   //00100000
#define I86_IDT_DESC_RING3 0x60   //01100000
#define I86_IDT_DESC_PRESENT 0x80 //10000000

#define DISPATCHER_ISR 0x80

typedef struct idt_descriptor
{
  uint16_t baseLo;

  uint16_t sel;
  uint8_t reserved;
  uint8_t flags;

  uint16_t baseHi;
} __attribute__((packed)) idt_descriptor;

typedef struct idtr
{
  uint16_t limit;

  uint32_t base;
} __attribute__((packed)) idtr;

/* Struct which aggregates many registers */
typedef struct interrupt_registers
{
  uint32_t ds;                                     // Data segment selector
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
  uint32_t int_no, err_code;                       // Interrupt number and error code (if applicable)
  uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically.
} __attribute__((packed)) interrupt_registers;

typedef void (*I86_IRQ_HANDLER)(interrupt_registers *registers);

void idt_init();

#endif
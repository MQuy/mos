#ifndef _REGS_H_INCLUDED
#define _REGS_H_INCLUDED
//****************************************************************************
//**
//**    regs.h
//**
//**	processor register structures and declarations. This interface abstracts
//**	register names behind a common, portable interface
//**
//****************************************************************************

//============================================================================
//    INTERFACE REQUIRED HEADERS
//============================================================================

#include <stdint.h>

//============================================================================
//    INTERFACE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================
//============================================================================
//    INTERFACE CLASS PROTOTYPES / EXTERNAL CLASS REFERENCES
//============================================================================
//============================================================================
//    INTERFACE STRUCTURES / UTILITY CLASSES
//============================================================================

//! 32 bit registers
typedef struct _R32BIT
{
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp, esp, eflags;
    uint8_t cflag;
} _R32BIT;

//! 16 bit registers
typedef struct _R16BIT
{
    uint16_t ax, bx, cx, dx, si, di, bp, sp, es, cs, ss, ds, flags;
    uint8_t cflag;
} _R16BIT;

//! 16 bit registers expressed in 32 bit registers
typedef struct _R16BIT32
{
    uint16_t ax, axh, bx, bxh, cx, cxh, dx, dxh;
    uint16_t si, di, bp, sp, es, cs, ss, ds, flags;
    uint8_t cflags;
} _R16BIT32;

//! 8 bit registers
typedef struct _R8BIT
{
    uint8_t al, ah, bl, bh, cl, ch, dl, dh;
} _R8BIT;

//! 8 bit registers expressed in 32 bit registers
typedef struct _R8BIT32
{
    uint8_t al, ah;
    uint16_t axh;
    uint8_t bl, bh;
    uint16_t bxh;
    uint8_t cl, ch;
    uint16_t cxh;
    uint8_t dl, dh;
    uint16_t dxh;
} _R8BIT32;

//! 8 and 16 bit registers union
typedef union _INTR16 {
    struct _R16BIT x;
    struct _R8BIT h;
} _INTR16;

//! 32 bit, 16 bit and 8 bit registers union
typedef union _INTR32 {
    struct _R32BIT x;
    struct _R16BIT32 l;
    struct _R8BIT32 h;
} _INTR32;

/* Struct which aggregates many registers */
typedef struct interrupt_registers
{
    uint32_t ds;                                     // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;                       // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically.
} __attribute__((packed)) interrupt_registers;

//! interrupt handler w/o error code
//! Note: interrupt handlers are called by the processor. The stack setup may change
//! so we leave it up to the interrupts' implimentation to handle it and properly return
typedef void (*I86_IRQ_HANDLER)(interrupt_registers *registers);

//============================================================================
//    INTERFACE DATA DECLARATIONS
//============================================================================
//============================================================================
//    INTERFACE FUNCTION PROTOTYPES
//============================================================================
//============================================================================
//    INTERFACE OBJECT CLASS DEFINITIONS
//============================================================================
//============================================================================
//    INTERFACE TRAILING HEADERS
//============================================================================
//****************************************************************************
//**
//**    END regs.h
//**
//****************************************************************************
#endif

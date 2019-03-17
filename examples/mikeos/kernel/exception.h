
#ifndef _EXCEPTION_H
#define _EXCEPTION_H
//****************************************************************************
//**
//**    exception.h
//**		system exception handlers. These are registered during system
//**		initialization and called automatically when they are encountered
//**
//****************************************************************************

#include <stdarg.h>

//============================================================================
//    INTERFACE REQUIRED HEADERS
//============================================================================
//============================================================================
//    INTERFACE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================
//============================================================================
//    INTERFACE CLASS PROTOTYPES / EXTERNAL CLASS REFERENCES
//============================================================================
//============================================================================
//    INTERFACE STRUCTURES / UTILITY CLASSES
//============================================================================
//============================================================================
//    INTERFACE DATA DECLARATIONS
//============================================================================
//============================================================================
//    INTERFACE FUNCTION PROTOTYPES
//============================================================================

//! exception handlers

//! divide by 0
void divide_by_zero_fault(unsigned int cs,
                          unsigned int eip, unsigned int eflags);

//! single step
void single_step_trap(unsigned int cs,
                      unsigned int eip, unsigned int eflags);

//! non maskable interrupt trap
void nmi_trap(unsigned int cs,
              unsigned int eip, unsigned int eflags);

//! breakpoint hit
void breakpoint_trap(unsigned int cs,
                     unsigned int eip, unsigned int eflags);

//! overflow
void overflow_trap(unsigned int cs,
                   unsigned int eip, unsigned int eflags);

//! bounds check
void bounds_check_fault(unsigned int cs,
                        unsigned int eip, unsigned int eflags);

//! invalid opcode / instruction
void invalid_opcode_fault(unsigned int cs,
                          unsigned int eip, unsigned int eflags);

//! device not available
void no_device_fault(unsigned int cs,
                     unsigned int eip, unsigned int eflags);

//! double fault
void double_fault_abort(unsigned int cs, unsigned int err,
                        unsigned int eip, unsigned int eflags);

//! invalid Task State Segment (TSS)
void invalid_tss_fault(unsigned int cs, unsigned int err,
                       unsigned int eip, unsigned int eflags);

//! segment not present
void no_segment_fault(unsigned int cs, unsigned int err,
                      unsigned int eip, unsigned int eflags);

//! stack fault
void stack_fault(unsigned int cs, unsigned int err,
                 unsigned int eip, unsigned int eflags);

//! general protection fault
void general_protection_fault(unsigned int cs, unsigned int err,
                              unsigned int eip, unsigned int eflags);

//! page fault
void page_fault(unsigned int cs, unsigned int err,
                unsigned int eip, unsigned int eflags);

//! Floating Point Unit (FPU) error
void fpu_fault(unsigned int cs,
               unsigned int eip, unsigned int eflags);

//! alignment check
void alignment_check_fault(unsigned int cs, unsigned int err,
                           unsigned int eip, unsigned int eflags);

//! machine check
void machine_check_abort(unsigned int cs,
                         unsigned int eip, unsigned int eflags);

//! Floating Point Unit (FPU) Single Instruction Multiple Data (SIMD) error
void simd_fpu_fault(unsigned int cs,
                    unsigned int eip, unsigned int eflags);

void kernel_panic(const char *fmt, ...);
//============================================================================
//    INTERFACE OBJECT CLASS DEFINITIONS
//============================================================================
//============================================================================
//    INTERFACE TRAILING HEADERS
//============================================================================
//****************************************************************************
//**
//**    END [exception.h]
//**
//****************************************************************************

#endif

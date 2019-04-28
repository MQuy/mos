#ifndef SYSTEM_EXCEPTION_H
#define SYSTEM_EXCEPTION_H

#include <stdarg.h>
#include <stdint.h>

//! exception handlers
void divide_by_zero_fault(interrupt_registers *regs);
void single_step_trap(interrupt_registers *regs);
void nmi_trap(interrupt_registers *regs);
void breakpoint_trap(interrupt_registers *regs);
void overflow_trap(interrupt_registers *regs);
void bounds_check_fault(interrupt_registers *regs);
void invalid_opcode_fault(interrupt_registers *regs);
void no_device_fault(interrupt_registers *regs);
void double_fault_abort(interrupt_registers *regs);
void invalid_tss_fault(interrupt_registers *regs);
void no_segment_fault(interrupt_registers *regs);
void stack_fault(interrupt_registers *regs);
void general_protection_fault(interrupt_registers *regs);
void page_fault(interrupt_registers *regs);
void fpu_fault(interrupt_registers *regs);
void alignment_check_fault(interrupt_registers *regs);
void machine_check_abort(interrupt_registers *regs);
void simd_fpu_fault(interrupt_registers *regs);

void exception_init();
void kernel_panic(const char *fmt, ...);

#endif

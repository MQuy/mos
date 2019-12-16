#ifndef SYSTEM_EXCEPTION_H
#define SYSTEM_EXCEPTION_H

#include <stdarg.h>
#include <stdint.h>

//! exception handlers
int32_t divide_by_zero_fault(interrupt_registers *regs);
int32_t single_step_trap(interrupt_registers *regs);
int32_t nmi_trap(interrupt_registers *regs);
int32_t breakpoint_trap(interrupt_registers *regs);
int32_t overflow_trap(interrupt_registers *regs);
int32_t bounds_check_fault(interrupt_registers *regs);
int32_t invalid_opcode_fault(interrupt_registers *regs);
int32_t no_device_fault(interrupt_registers *regs);
int32_t double_fault_abort(interrupt_registers *regs);
int32_t invalid_tss_fault(interrupt_registers *regs);
int32_t no_segment_fault(interrupt_registers *regs);
int32_t stack_fault(interrupt_registers *regs);
int32_t general_protection_fault(interrupt_registers *regs);
int32_t page_fault(interrupt_registers *regs);
int32_t fpu_fault(interrupt_registers *regs);
int32_t alignment_check_fault(interrupt_registers *regs);
int32_t machine_check_abort(interrupt_registers *regs);
int32_t simd_fpu_fault(interrupt_registers *regs);

void exception_init();
void kernel_panic(const char *fmt, ...);

#endif

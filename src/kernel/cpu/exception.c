#include "exception.h"

#include <cpu/hal.h>
#include <cpu/idt.h>
#include <shared/cdefs.h>
#include <utils/printf.h>

//! something is wrong--bail out
static void kernel_panic(const char *fmt, ...)
{
	disable_interrupts();

	va_list args;
	va_start(args, fmt);
	va_end(args);

	DEBUG &&debug_println(DEBUG_FATAL, fmt, args);

	for (;;)
		;
}

static int32_t divide_by_zero_fault(struct interrupt_registers *regs)
{
	kernel_panic("Divide by 0");
	return IRQ_HANDLER_STOP;
}

static int32_t single_step_trap(struct interrupt_registers *regs)
{
	kernel_panic("Single step");
	return IRQ_HANDLER_STOP;
}

static int32_t nmi_trap(struct interrupt_registers *regs)
{
	kernel_panic("NMI trap");
	return IRQ_HANDLER_STOP;
}

static int32_t breakpoint_trap(struct interrupt_registers *regs)
{
	kernel_panic("Breakpoint trap");
	return IRQ_HANDLER_STOP;
}

static int32_t overflow_trap(struct interrupt_registers *regs)
{
	kernel_panic("Overflow trap");
	return IRQ_HANDLER_STOP;
}

static int32_t bounds_check_fault(struct interrupt_registers *regs)
{
	kernel_panic("Bounds check fault");
	return IRQ_HANDLER_STOP;
}

static int32_t invalid_opcode_fault(struct interrupt_registers *regs)
{
	kernel_panic("Invalid opcode");
	return IRQ_HANDLER_STOP;
}

static int32_t no_device_fault(struct interrupt_registers *regs)
{
	kernel_panic("Device not found");
	return IRQ_HANDLER_STOP;
}

static int32_t double_fault_abort(struct interrupt_registers *regs)
{
	kernel_panic("Double fault");
	return IRQ_HANDLER_STOP;
}

static int32_t invalid_tss_fault(struct interrupt_registers *regs)
{
	kernel_panic("Invalid TSS");
	return IRQ_HANDLER_STOP;
}

static int32_t no_segment_fault(struct interrupt_registers *regs)
{
	kernel_panic("Invalid segment");
	return IRQ_HANDLER_STOP;
}

static int32_t stack_fault(struct interrupt_registers *regs)
{
	kernel_panic("Stack fault");
	return IRQ_HANDLER_STOP;
}

static int32_t general_protection_fault(struct interrupt_registers *regs)
{
	kernel_panic("General Protection Fault");
	return IRQ_HANDLER_STOP;
}

static int32_t page_fault(__unused struct interrupt_registers *regs)
{
	// uint32_t faultAddr = 0;
	// int error_code = regs->err_code;

	// __asm__ __volatile__("mov %%cr2, %%eax	\n"
	// 										 "mov %%eax, %0			\n"
	// 										 : "=r"(faultAddr));

	// DebugPrintf("\nPage Fault at 0x%x", faultAddr);
	// DebugPrintf("\nReason: %s, %s, %s%s%s",
	// 						error_code & 0b1 ? "protection violation" : "non-present page",
	// 						error_code & 0b10 ? "write" : "read",
	// 						error_code & 0b100 ? "user mode" : "supervisor mode",
	// 						error_code & 0b1000 ? ", reserved" : "",
	// 						error_code & 0b10000 ? ", instruction fetch" : "");

	for (;;)
		;
	return IRQ_HANDLER_STOP;
}

static int32_t fpu_fault(struct interrupt_registers *regs)
{
	kernel_panic("FPU Fault");
	return IRQ_HANDLER_STOP;
}

static int32_t alignment_check_fault(struct interrupt_registers *regs)
{
	kernel_panic("Alignment Check");
	return IRQ_HANDLER_STOP;
}

static int32_t machine_check_abort(struct interrupt_registers *regs)
{
	kernel_panic("Machine Check");
	return IRQ_HANDLER_STOP;
}

static int32_t simd_fpu_fault(struct interrupt_registers *regs)
{
	kernel_panic("FPU SIMD fault");
	return IRQ_HANDLER_STOP;
}

void exception_init()
{
	DEBUG &&debug_println(DEBUG_INFO, "[exception] - Initializing");

	register_interrupt_handler(0, (I86_IRQ_HANDLER)divide_by_zero_fault);
	register_interrupt_handler(1, (I86_IRQ_HANDLER)single_step_trap);
	register_interrupt_handler(2, (I86_IRQ_HANDLER)nmi_trap);
	register_interrupt_handler(3, (I86_IRQ_HANDLER)breakpoint_trap);
	register_interrupt_handler(4, (I86_IRQ_HANDLER)overflow_trap);
	register_interrupt_handler(5, (I86_IRQ_HANDLER)bounds_check_fault);
	register_interrupt_handler(6, (I86_IRQ_HANDLER)invalid_opcode_fault);
	register_interrupt_handler(7, (I86_IRQ_HANDLER)no_device_fault);
	register_interrupt_handler(8, (I86_IRQ_HANDLER)double_fault_abort);
	register_interrupt_handler(10, (I86_IRQ_HANDLER)invalid_tss_fault);
	register_interrupt_handler(11, (I86_IRQ_HANDLER)no_segment_fault);
	register_interrupt_handler(12, (I86_IRQ_HANDLER)stack_fault);
	register_interrupt_handler(13, (I86_IRQ_HANDLER)general_protection_fault);
	register_interrupt_handler(14, (I86_IRQ_HANDLER)page_fault);
	register_interrupt_handler(16, (I86_IRQ_HANDLER)fpu_fault);
	register_interrupt_handler(17, (I86_IRQ_HANDLER)alignment_check_fault);
	register_interrupt_handler(18, (I86_IRQ_HANDLER)machine_check_abort);
	register_interrupt_handler(19, (I86_IRQ_HANDLER)simd_fpu_fault);

	DEBUG &&debug_println(DEBUG_INFO, "[exception] - Done");
}

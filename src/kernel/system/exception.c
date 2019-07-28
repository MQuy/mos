#include <kernel/graphics/DebugDisplay.h>
#include <kernel/cpu/hal.h>
#include <kernel/cpu/idt.h>
#include "exception.h"

void divide_by_zero_fault(interrupt_registers *regs)
{
	kernel_panic("Divide by 0");
}

void single_step_trap(interrupt_registers *regs)
{
	kernel_panic("Single step");
}

void nmi_trap(interrupt_registers *regs)
{
	kernel_panic("NMI trap");
}

void breakpoint_trap(interrupt_registers *regs)
{
	kernel_panic("Breakpoint trap");
}

void overflow_trap(interrupt_registers *regs)
{
	kernel_panic("Overflow trap");
}

void bounds_check_fault(interrupt_registers *regs)
{
	kernel_panic("Bounds check fault");
}

void invalid_opcode_fault(interrupt_registers *regs)
{
	kernel_panic("Invalid opcode");
}

void no_device_fault(interrupt_registers *regs)
{
	kernel_panic("Device not found");
}

void double_fault_abort(interrupt_registers *regs)
{
	kernel_panic("Double fault");
}

void invalid_tss_fault(interrupt_registers *regs)
{
	kernel_panic("Invalid TSS");
}

void no_segment_fault(interrupt_registers *regs)
{
	kernel_panic("Invalid segment");
}

void stack_fault(interrupt_registers *regs)
{
	kernel_panic("Stack fault");
}

void general_protection_fault(interrupt_registers *regs)
{
	kernel_panic("General Protection Fault");
}

void page_fault(interrupt_registers *regs)
{
	int faultAddr = 0;
	int error_code = regs->err_code;

	__asm__ __volatile__("mov %%cr2, %%eax	\n"
											 "mov %%eax, %0			\n"
											 : "=r"(faultAddr));

	DebugPrintf("\nPage Fault at 0x%x", faultAddr);
	DebugPrintf("\nReason: %s, %s, %s%s%s",
							error_code & 0b1 ? "protection violation" : "non-present page",
							error_code & 0b10 ? "write" : "read",
							error_code & 0b100 ? "user mode" : "supervisor mode",
							error_code & 0b1000 ? ", reserved" : "",
							error_code & 0b10000 ? ", instruction fetch" : "");
	for (;;)
		;
}

void fpu_fault(interrupt_registers *regs)
{
	kernel_panic("FPU Fault");
}

void alignment_check_fault(interrupt_registers *regs)
{
	kernel_panic("Alignment Check");
}

void machine_check_abort(interrupt_registers *regs)
{
	kernel_panic("Machine Check");
}

void simd_fpu_fault(interrupt_registers *regs)
{
	kernel_panic("FPU SIMD fault");
}

static char *sickpc = " \
                               _______      \n\
                               |.-----.|    \n\
                               ||x . x||    \n\
                               ||_.-._||    \n\
                               `--)-(--`    \n\
                              __[=== o]___  \n\
                             |:::::::::::|\\ \n\
                             `-=========-`()\n\
                                M. O. S.\n\n";

//! something is wrong--bail out
void kernel_panic(const char *fmt, ...)
{
	disable_interrupts();

	va_list args;
	va_start(args, fmt);
	va_end(args);

	char *disclamer = "We apologize, MOS has encountered a problem and has been shut down\n\
to prevent damage to your computer. Any unsaved work might be lost.\n\
We are sorry for the inconvenience this might have caused.\n\n\
Please report the following information and restart your computer.\n\
The system has been halted.\n\n";

	DebugClrScr(0x1f);
	DebugGotoXY(0, 0);
	DebugSetColor(0x1f);
	DebugPuts(sickpc);
	DebugPuts(disclamer);

	DebugPrintf("*** STOP: ");
	DebugPrintf(fmt, args);

	for (;;)
		;
}

void exception_init()
{
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
}
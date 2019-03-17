//****************************************************************************
//**
//**    exception.cpp
//**		system exception handlers. These are registered during system
//**		initialization and called automatically when they are encountered
//**
//****************************************************************************

#include "exception.h"
#include "DebugDisplay.h"
#include "./Hal/Hal.h"

//! For now, all of these interrupt handlers just disable hardware interrupts
//! and calls kernal_panic(). This displays an error and halts the system

//! divide by 0 fault
void divide_by_zero_fault(unsigned int cs, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Divide by 0");
	for (;;)
		;
}

//! single step
void single_step_trap(unsigned int cs, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Single step");
	for (;;)
		;
}

//! non maskable  trap
void nmi_trap(unsigned int cs, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("NMI trap");
	for (;;)
		;
}

//! breakpoint hit
void breakpoint_trap(unsigned int cs, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Breakpoint trap");
	for (;;)
		;
}

//! overflow
void overflow_trap(unsigned int cs, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Overflow trap");
	for (;;)
		;
}

//! bounds check
void bounds_check_fault(unsigned int cs, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Bounds check fault");
	for (;;)
		;
}

//! invalid opcode / instruction
void invalid_opcode_fault(unsigned int cs, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Invalid opcode");
	for (;;)
		;
}

//! device not available
void no_device_fault(unsigned int cs, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Device not found");
	for (;;)
		;
}

//! double fault
void double_fault_abort(unsigned int cs, unsigned int err, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Double fault");
	for (;;)
		;
}

//! invalid Task State Segment (TSS)
void invalid_tss_fault(unsigned int cs, unsigned int err, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Invalid TSS");
	for (;;)
		;
}

//! segment not present
void no_segment_fault(unsigned int cs, unsigned int err, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Invalid segment");
	for (;;)
		;
}

//! stack fault
void stack_fault(unsigned int cs, unsigned int err, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Stack fault");
	for (;;)
		;
}

//! general protection fault
void general_protection_fault(unsigned int cs, unsigned int err, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("General Protection Fault");
	for (;;)
		;
}

//! page fault
void page_fault(unsigned int cs, unsigned int err, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Page Fault");
	for (;;)
		;
}

//! Floating Point Unit (FPU) error
void fpu_fault(unsigned int cs, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("FPU Fault");
	for (;;)
		;
}

//! alignment check
void alignment_check_fault(unsigned int cs, unsigned int err, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Alignment Check");
	for (;;)
		;
}

//! machine check
void machine_check_abort(unsigned int cs, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("Machine Check");
	for (;;)
		;
}

//! Floating Point Unit (FPU) Single Instruction Multiple Data (SIMD) error
void simd_fpu_fault(unsigned int cs, unsigned int eip, unsigned int eflags)
{

	//	intstart ();
	kernel_panic("FPU SIMD fault");
	for (;;)
		;
}

//! something is wrong--bail out
void kernel_panic(const char *fmt, ...)
{

	disable();

	va_list args;
	va_start(args, fmt);

	// We will need a vsprintf() here. I will see if I can write
	// one before the tutorial release

	va_end(args);

	char *disclamer = "We apologize, MOS has encountered a problem and has been shut down\n\
to prevent damage to your computer. Any unsaved work might be lost.\n\
We are sorry for the inconvenience this might have caused.\n\n\
Please report the following information and restart your computer.\n\
The system has been halted.\n\n";

	DebugClrScr(0x1f);
	DebugGotoXY(0, 0);
	DebugSetColor(0x1f);
	DebugPuts(disclamer);

	DebugPrintf("*** STOP: %s", fmt);

	for (;;)
		;
}
//****************************************************************************
//**
//**    Idt.cpp
//**		Interrupt Descriptor Table. The IDT is responsible for providing
//**	the interface for managing interrupts, installing, setting, requesting,
//**	generating, and interrupt callback managing.
//**
//****************************************************************************

//============================================================================
//    IMPLEMENTATION HEADERS
//============================================================================

#include "idt.h"
#include "pic.h"
#include "Hal.h"
#include "pit.h"
#include "../Include/string.h"
#include "../DebugDisplay.h"

//============================================================================
//    IMPLEMENTATION PRIVATE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE CLASS PROTOTYPES / EXTERNAL CLASS REFERENCES
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE STRUCTURES / UTILITY CLASSES
//============================================================================

//============================================================================
//    IMPLEMENTATION REQUIRED EXTERNAL REFERENCES (AVOID)
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE DATA
//============================================================================

//! interrupt descriptor table
static idt_descriptor _idt[I86_MAX_INTERRUPTS];

//! idtr structure used to help define the cpu's idtr register
static idtr _idtr;

I86_IRQ_HANDLER interrupt_handlers[256];
//============================================================================
//    INTERFACE DATA
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTION PROTOTYPES
//============================================================================

//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTIONS
//============================================================================

//! installs idtr into processors idtr register
static void idt_install()
{
	__asm__ __volatile__("lidtl (%0)"
											 :
											 : "r"(&_idtr));
}

//! default handler to catch unhandled system interrupts.
void i86_default_handler(interrupt_registers *registers)
{

	//! clear interrupts to prevent double fault
	disable();

	uint8_t int_no = registers->int_no & 0xFF;

	//! print debug message and halt
	DebugPrintf("\n*** [i86 Hal]: unhandled exception %d", int_no);

	for (;;)
		;
}

//============================================================================
//    INTERFACE FUNCTIONS
//============================================================================

//! returns interrupt descriptor
idt_descriptor *i86_get_ir(uint32_t i)
{

	if (i > I86_MAX_INTERRUPTS)
		return 0;

	return &_idt[i];
}

//! installs a new interrupt handler
int i86_install_ir(uint32_t i, uint16_t flags, uint16_t sel, I86_IRQ_HANDLER irq)
{

	if (i > I86_MAX_INTERRUPTS)
		return 0;

	if (!irq)
		return 0;

	uint64_t uiBase = (uint64_t) & (*irq);

	//! store base address into idt
	_idt[i].baseLo = (uint16_t)(uiBase & 0xffff);
	_idt[i].baseHi = (uint16_t)((uiBase >> 16) & 0xffff);
	_idt[i].reserved = 0;
	_idt[i].flags = (uint8_t)(flags);
	_idt[i].sel = sel;

	return 0;
}

//! initialize idt
int i86_idt_initialize(uint16_t codeSel)
{

	//! set up idtr for processor
	_idtr.limit = sizeof(struct idt_descriptor) * I86_MAX_INTERRUPTS - 1;
	_idtr.base = (uint32_t)&_idt[0];

	//! null out the idt
	memset((void *)&_idt[0], 0, sizeof(idt_descriptor) * I86_MAX_INTERRUPTS - 1);

	//! register default handlers
	for (int i = 0; i < I86_MAX_INTERRUPTS; i++)
		i86_install_ir(i, I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32,
									 codeSel, (I86_IRQ_HANDLER)i86_default_handler);

	setvect(0, (I86_IRQ_HANDLER)isr0);
	setvect(1, (I86_IRQ_HANDLER)isr1);
	setvect(2, (I86_IRQ_HANDLER)isr2);
	setvect(3, (I86_IRQ_HANDLER)isr3);
	setvect(4, (I86_IRQ_HANDLER)isr4);
	setvect(5, (I86_IRQ_HANDLER)isr5);
	setvect(6, (I86_IRQ_HANDLER)isr6);
	setvect(7, (I86_IRQ_HANDLER)isr7);
	setvect(8, (I86_IRQ_HANDLER)isr8);
	setvect(9, (I86_IRQ_HANDLER)isr9);
	setvect(10, (I86_IRQ_HANDLER)isr10);
	setvect(11, (I86_IRQ_HANDLER)isr11);
	setvect(12, (I86_IRQ_HANDLER)isr12);
	setvect(13, (I86_IRQ_HANDLER)isr13);
	setvect(14, (I86_IRQ_HANDLER)isr14);
	setvect(15, (I86_IRQ_HANDLER)isr15);
	setvect(16, (I86_IRQ_HANDLER)isr16);
	setvect(17, (I86_IRQ_HANDLER)isr17);
	setvect(18, (I86_IRQ_HANDLER)isr18);
	setvect(19, (I86_IRQ_HANDLER)isr19);
	setvect(20, (I86_IRQ_HANDLER)isr20);
	setvect(21, (I86_IRQ_HANDLER)isr21);
	setvect(22, (I86_IRQ_HANDLER)isr22);
	setvect(23, (I86_IRQ_HANDLER)isr23);
	setvect(24, (I86_IRQ_HANDLER)isr24);
	setvect(25, (I86_IRQ_HANDLER)isr25);
	setvect(26, (I86_IRQ_HANDLER)isr26);
	setvect(27, (I86_IRQ_HANDLER)isr27);
	setvect(28, (I86_IRQ_HANDLER)isr28);
	setvect(29, (I86_IRQ_HANDLER)isr29);
	setvect(30, (I86_IRQ_HANDLER)isr30);
	setvect(31, (I86_IRQ_HANDLER)isr31);

	// Install the IRQs
	setvect(32, (I86_IRQ_HANDLER)irq0);
	setvect(33, (I86_IRQ_HANDLER)irq1);
	setvect(34, (I86_IRQ_HANDLER)irq2);
	setvect(35, (I86_IRQ_HANDLER)irq3);
	setvect(36, (I86_IRQ_HANDLER)irq4);
	setvect(37, (I86_IRQ_HANDLER)irq5);
	setvect(38, (I86_IRQ_HANDLER)irq6);
	setvect(39, (I86_IRQ_HANDLER)irq7);
	setvect(40, (I86_IRQ_HANDLER)irq8);
	setvect(41, (I86_IRQ_HANDLER)irq9);
	setvect(42, (I86_IRQ_HANDLER)irq10);
	setvect(43, (I86_IRQ_HANDLER)irq11);
	setvect(44, (I86_IRQ_HANDLER)irq12);
	setvect(45, (I86_IRQ_HANDLER)irq13);
	setvect(46, (I86_IRQ_HANDLER)irq14);
	setvect(47, (I86_IRQ_HANDLER)irq15);

	//! install our idt
	idt_install();

	return 0;
}

void register_interrupt_handler(uint8_t n, I86_IRQ_HANDLER handler)
{
	interrupt_handlers[n] = handler;
}

void isr_wrapper_handler(interrupt_registers *r)
{
	uint8_t int_no = r->int_no & 0xFF;
	I86_IRQ_HANDLER handler = interrupt_handlers[int_no];
	if (handler != 0)
	{
		handler(r);
	}
	else
	{
		DebugPrintf("\nUnhandled interrupt: %d", int_no);
		for (;;)
			;
	}
}

void irq_wrapper_handler(interrupt_registers *registers)
{
	/* After every interrupt we need to send an EOI to the PICs
	   * or they will not send another interrupt again */
	if (registers->int_no >= 40)
		i86_pic_send_command(I86_PIC_OCW2_MASK_EOI, 1);
	i86_pic_send_command(I86_PIC_OCW2_MASK_EOI, 0);

	/* Handle the interrupt in a more modular way */
	I86_IRQ_HANDLER handler = interrupt_handlers[registers->int_no];
	if (handler != 0)
	{
		handler(registers);
	}
}

//============================================================================
//    INTERFACE CLASS BODIES
//============================================================================
//****************************************************************************
//**
//**    END[idt.cpp]
//**
//****************************************************************************

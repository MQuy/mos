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
int i86_install_ir(uint32_t i, uint16_t flags, uint16_t sel, uint32_t irq)
{

	if (i > I86_MAX_INTERRUPTS)
		return 0;

	if (!irq)
		return 0;

	//! store base address into idt
	_idt[i].baseLo = (uint16_t)(irq & 0xffff);
	_idt[i].baseHi = (uint16_t)((irq >> 16) & 0xffff);
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

	setvect(0, (uint32_t)isr0);
	setvect(1, (uint32_t)isr1);
	setvect(2, (uint32_t)isr2);
	setvect(3, (uint32_t)isr3);
	setvect(4, (uint32_t)isr4);
	setvect(5, (uint32_t)isr5);
	setvect(6, (uint32_t)isr6);
	setvect(7, (uint32_t)isr7);
	setvect(8, (uint32_t)isr8);
	setvect(9, (uint32_t)isr9);
	setvect(10, (uint32_t)isr10);
	setvect(11, (uint32_t)isr11);
	setvect(12, (uint32_t)isr12);
	setvect(13, (uint32_t)isr13);
	setvect(14, (uint32_t)isr14);
	setvect(15, (uint32_t)isr15);
	setvect(16, (uint32_t)isr16);
	setvect(17, (uint32_t)isr17);
	setvect(18, (uint32_t)isr18);
	setvect(19, (uint32_t)isr19);
	setvect(20, (uint32_t)isr20);
	setvect(21, (uint32_t)isr21);
	setvect(22, (uint32_t)isr22);
	setvect(23, (uint32_t)isr23);
	setvect(24, (uint32_t)isr24);
	setvect(25, (uint32_t)isr25);
	setvect(26, (uint32_t)isr26);
	setvect(27, (uint32_t)isr27);
	setvect(28, (uint32_t)isr28);
	setvect(29, (uint32_t)isr29);
	setvect(30, (uint32_t)isr30);
	setvect(31, (uint32_t)isr31);

	// Install the IRQs
	setvect(32, (uint32_t)irq0);
	setvect(33, (uint32_t)irq1);
	setvect(34, (uint32_t)irq2);
	setvect(35, (uint32_t)irq3);
	setvect(36, (uint32_t)irq4);
	setvect(37, (uint32_t)irq5);
	setvect(38, (uint32_t)irq6);
	setvect(39, (uint32_t)irq7);
	setvect(40, (uint32_t)irq8);
	setvect(41, (uint32_t)irq9);
	setvect(42, (uint32_t)irq10);
	setvect(43, (uint32_t)irq11);
	setvect(44, (uint32_t)irq12);
	setvect(45, (uint32_t)irq13);
	setvect(46, (uint32_t)irq14);
	setvect(47, (uint32_t)irq15);

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

//****************************************************************************
//**
//**    cpu.cpp
//**
//**	This is the processor interface. Everything outside of this module
//**	must use this interface when working on processor data.
//**
//**	A processor is a module that manages the very basic data structures
//**	and data within the system. The processor interface provides the interface
//**	for managing processors, processor cores, accessing processor structures,
//**	and more
//**
//****************************************************************************

//============================================================================
//    IMPLEMENTATION HEADERS
//============================================================================

#include "cpu.h"
#include "gdt.h"
#include "idt.h"

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
//============================================================================
//    INTERFACE DATA
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTION PROTOTYPES
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTIONS
//============================================================================
//============================================================================
//    INTERFACE FUNCTIONS
//============================================================================

//! Initializes cpu resources
int i86_cpu_initialize()
{

	//! initialize processor tables
	i86_gdt_initialize();
	i86_idt_initialize(0x8);

	return 0;
}

//! shuts down cpu resources...Nothing to do yet
void i86_cpu_shutdown()
{
}

//! returns vender name of cpu
char *i86_cpu_get_vender()
{

	static char vender[32] = {0};
	__asm__ __volatile__("movl $0, %%eax;\n"
											 "cpuid;\n"
											 "leal (%0), %%eax;\n"
											 "movl %%ebx, (%%eax);\n"
											 "movl %%edx, 0x4(%%eax);\n"
											 "movl %%ecx, 0x8(%%eax)"
											 : "=m"(vender)
											 :
											 :);
	return vender;
}

//! flush all internal and external processor caches
void i86_cpu_flush_caches()
{

#ifdef _MSC_VER
	_asm {
		cli
		invd
		sti
	}
#endif
}

//! same as above but writes the data back into memory first
void i86_cpu_flush_caches_write()
{

#ifdef _MSC_VER
	_asm {
		cli
		wbinvd
		sti
	}
#endif
}

//! flushes TLB entry
void i86_cpu_flush_tlb_entry(uint32_t addr)
{

#ifdef _MSC_VER
	_asm {
		cli
		invlpg	addr
		sti
	}
#endif
}

//============================================================================
//    INTERFACE CLASS BODIES
//============================================================================
//****************************************************************************
//**
//**    END[String.cpp]
//**
//****************************************************************************

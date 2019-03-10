#ifndef _MMNGR_VIRT_PDE_H
#define _MMNGR_VIRT_PDE_H
//****************************************************************************
//**
//**    vmmngr_pde.h
//**		-Page Directory Entries (PDE). This provides an abstract interface
//**	to aid in management of PDEs.
//**
//****************************************************************************
//============================================================================
//    INTERFACE REQUIRED HEADERS
//============================================================================

#include <stdint.h>
#include "mmngr_phys.h" //physical_addr

//============================================================================
//    INTERFACE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================

//! this format is defined by the i86 architecture--be careful if you modify it
enum PAGE_PDE_FLAGS
{

	I86_PDE_PRESENT = 1,				//0000000000000000000000000000001
	I86_PDE_WRITABLE = 2,				//0000000000000000000000000000010
	I86_PDE_USER = 4,						//0000000000000000000000000000100
	I86_PDE_PWT = 8,						//0000000000000000000000000001000
	I86_PDE_PCD = 0x10,					//0000000000000000000000000010000
	I86_PDE_ACCESSED = 0x20,		//0000000000000000000000000100000
	I86_PDE_DIRTY = 0x40,				//0000000000000000000000001000000
	I86_PDE_4MB = 0x80,					//0000000000000000000000010000000
	I86_PDE_CPU_GLOBAL = 0x100, //0000000000000000000000100000000
	I86_PDE_LV4_GLOBAL = 0x200, //0000000000000000000001000000000
	I86_PDE_FRAME = 0x7FFFF000	//1111111111111111111000000000000
};

//! a page directery entry
typedef uint32_t pd_entry;

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

//! sets a flag in the page table entry
void pd_entry_add_attrib(pd_entry *e, uint32_t attrib);

//! clears a flag in the page table entry
void pd_entry_del_attrib(pd_entry *e, uint32_t attrib);

//! sets a frame to page table entry
void pd_entry_set_frame(pd_entry *, physical_addr);

//! test if page is present
bool pd_entry_is_present(pd_entry e);

//! test if directory is user mode
bool pd_entry_is_user(pd_entry);

//! test if directory contains 4mb pages
bool pd_entry_is_4mb(pd_entry);

//! test if page is writable
bool pd_entry_is_writable(pd_entry e);

//! get page table entry frame address
physical_addr pd_entry_pfn(pd_entry e);

//! enable global pages
void pd_entry_enable_global(pd_entry e);

//============================================================================
//    INTERFACE OBJECT CLASS DEFINITIONS
//============================================================================
//============================================================================
//    INTERFACE TRAILING HEADERS
//============================================================================
//****************************************************************************
//**
//**    END [vmmngr_pde.h]
//**
//****************************************************************************

#endif

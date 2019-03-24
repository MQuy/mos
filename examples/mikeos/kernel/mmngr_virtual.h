#ifndef _MMNGR_VIRT_H
#define _MMNGR_VIRT_H
//****************************************************************************
//**
//**    mmngr_virtual.h
//**		-Virtual Memory Manager
//**
//****************************************************************************
//============================================================================
//    INTERFACE REQUIRED HEADERS
//============================================================================

#include <stdint.h>
#include "vmmngr_pte.h"
#include "vmmngr_pde.h"

//============================================================================
//    INTERFACE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================

//! virtual address
typedef uint32_t virtual_addr;

//! i86 architecture defines 1024 entries per table--do not change
#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR 1024

//============================================================================
//    INTERFACE CLASS PROTOTYPES / EXTERNAL CLASS REFERENCES
//============================================================================
//============================================================================
//    INTERFACE STRUCTURES / UTILITY CLASSES
//============================================================================

//! page table
typedef struct ptable
{

	pt_entry m_entries[PAGES_PER_TABLE];
} ptable;

//! page directory
typedef struct pdirectory
{

	pd_entry m_entries[PAGES_PER_DIR];
} pdirectory;

//============================================================================
//    INTERFACE DATA DECLARATIONS
//============================================================================
//============================================================================
//    INTERFACE FUNCTION PROTOTYPES
//============================================================================

//! initialize the memory manager
void vmmngr_initialize();

//! allocates a page in physical memory
bool vmmngr_alloc_page(pt_entry *);

//! frees a page in physical memory
void vmmngr_free_page(pt_entry *e);

//! switch to a new page directory
bool vmmngr_switch_pdirectory(pdirectory *);

//! get current page directory
pdirectory *vmmngr_get_directory();

//! flushes a cached translation lookaside buffer (TLB) entry
void vmmngr_flush_tlb_entry(virtual_addr addr);

//! clears a page table
void vmmngr_ptable_clear(ptable *p);

//! convert virtual address to page table index
uint32_t vmmngr_ptable_virt_to_index(virtual_addr addr);

//! get page entry from page table
pt_entry *vmmngr_ptable_lookup_entry(ptable *p, virtual_addr addr);

//! convert virtual address to page directory index
uint32_t vmmngr_pdirectory_virt_to_index(virtual_addr addr);

//! clears a page directory table
void vmmngr_pdirectory_clear(pdirectory *dir);

//! get directory entry from directory table
pd_entry *vmmngr_pdirectory_lookup_entry(pdirectory *p, virtual_addr addr);

/*
	New additions for chapter 24
*/

int vmmngr_createPageTable(pdirectory *dir, uint32_t virt, uint32_t flags);
void vmmngr_mapPhysicalAddress(pdirectory *dir, uint32_t virt, uint32_t phys, uint32_t flags);
void vmmngr_unmapPageTable(pdirectory *dir, uint32_t virt);
void vmmngr_unmapPhysicalAddress(pdirectory *dir, uint32_t virt);
pdirectory *vmmngr_createAddressSpace();
void *vmmngr_getPhysicalAddress(pdirectory *dir, uint32_t virt);

//============================================================================
//    INTERFACE OBJECT CLASS DEFINITIONS
//============================================================================
//============================================================================
//    INTERFACE TRAILING HEADERS
//============================================================================
//****************************************************************************
//**
//**    END [mmngr_virtual.h]
//**
//****************************************************************************

#endif

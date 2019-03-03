
#ifndef _MMNGR_PHYS_H
#define _MMNGR_PHYS_H
//****************************************************************************
//**
//**    mmngr_phys.cpp
//**		-Physical Memory Manager
//**
//****************************************************************************
//============================================================================
//    INTERFACE REQUIRED HEADERS
//============================================================================

#include <stdbool.h>
#include <stdint.h>
#include "stddef.h"

//============================================================================
//    INTERFACE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================

//! physical address
typedef uint32_t physical_addr;

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

//! initialize the physical memory manager
void pmmngr_init(size_t, physical_addr);

//! enables a physical memory region for use
void pmmngr_init_region(physical_addr, size_t);

//! disables a physical memory region as in use (unuseable)
void pmmngr_deinit_region(physical_addr base, size_t);

//! allocates a single memory block
void *pmmngr_alloc_block();

//! releases a memory block
void pmmngr_free_block(void *);

//! allocates blocks of memory
void *pmmngr_alloc_blocks(size_t);

//! frees blocks of memory
void pmmngr_free_blocks(void *, size_t);

//! returns amount of physical memory the manager is set to use
size_t pmmngr_get_memory_size();

//! returns number of blocks currently in use
uint32_t pmmngr_get_use_block_count();

//! returns number of blocks not in use
uint32_t pmmngr_get_free_block_count();

//! returns number of memory blocks
uint32_t pmmngr_get_block_count();

//! returns default memory block size in bytes
uint32_t pmmngr_get_block_size();

//! enable or disable paging
void pmmngr_paging_enable(bool);

//! test if paging is enabled
bool pmmngr_is_paging();

//! loads the page directory base register (PDBR)
void pmmngr_load_PDBR(physical_addr);

//! get PDBR physical address
physical_addr pmmngr_get_PDBR();

//============================================================================
//    INTERFACE OBJECT CLASS DEFINITIONS
//============================================================================
//============================================================================
//    INTERFACE TRAILING HEADERS
//============================================================================
//****************************************************************************
//**
//**    END [mmngr_phys.h]
//**
//****************************************************************************

#endif

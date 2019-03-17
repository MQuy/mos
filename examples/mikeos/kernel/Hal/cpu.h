#ifndef _CPU_H_INCLUDED
#define _CPU_H_INCLUDED
//****************************************************************************
//**
//**    cpu.h
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
//    INTERFACE REQUIRED HEADERS
//============================================================================

#include <stdint.h>
#include "regs.h"

//============================================================================
//    INTERFACE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================
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

//! initialize the processors
int i86_cpu_initialize();

//! shutdown the processors
void i86_cpu_shutdown();

//! get processor vender
char *i86_cpu_get_vender();

//! flush all internal and external processor caches
void i86_cpu_flush_caches();

//! same as above but writes the data back into memory first
void i86_cpu_flush_caches_write();

//! flushes translation lookaside buffer (TLB) entry
void i86_cpu_flush_tlb_entry(uint32_t);

//============================================================================
//    INTERFACE OBJECT CLASS DEFINITIONS
//============================================================================
//============================================================================
//    INTERFACE TRAILING HEADERS
//============================================================================
//****************************************************************************
//**
//**    END [FILE NAME]
//**
//****************************************************************************
#endif

#ifndef _HAL_H
#define _HAL_H
//****************************************************************************
//**
//**    Hal.h
//**		Hardware Abstraction Layer Interface
//**
//**	The Hardware Abstraction Layer (HAL) provides an abstract interface
//**	to control the basic motherboard hardware devices. This is accomplished
//**	by abstracting hardware dependencies behind this interface.
//**
//**	All routines and types are declared extern and must be defined within
//**	external libraries to define specific hal implimentations.
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

#define interrupt
#define far
#define near

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

//! initialize hardware abstraction layer
int hal_initialize();

//! shutdown hardware abstraction layer
int hal_shutdown();

//! enables hardware device interrupts
void enable();

//! disables hardware device interrupts
void disable();

//! generates interrupt
void geninterrupt(int n);

//! reads from hardware device port
unsigned char inportb(unsigned short id);

//! writes byte to hardware port
void outportb(unsigned short id, unsigned char value);

//! sets new interrupt vector
void setvect(int intno, I86_IRQ_HANDLER vect);

//! returns current interrupt at interrupt vector
void(far *getvect(int intno))();

//! notifies hal the interrupt is done
void interruptdone(unsigned int intno);

//! generates sound
void sound(unsigned frequency);

//! returns cpu vender
const char *get_cpu_vender();

//! returns current tick count (Only for demo)
int get_tick_count();

//============================================================================
//    INTERFACE OBJECT CLASS DEFINITIONS
//============================================================================
//============================================================================
//    INTERFACE TRAILING HEADERS
//============================================================================
//****************************************************************************
//**
//**    END [Hal.h]
//**
//****************************************************************************
#endif

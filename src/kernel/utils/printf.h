#ifndef LIBC_PRINTF_H
#define LIBC_PRINTF_H
//****************************************************************************
//**
//**    DebugDisplay.h
//**    - Provides display capabilities for debugging. Because it is
//**	  specifically for debugging and not final release, we don't
//** 	  care for portability here
//**
//****************************************************************************

//============================================================================
//    INTERFACE REQUIRED HEADERS
//============================================================================

#include <stdarg.h>
#include <stdint.h>

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

enum debug_level
{
  DEBUG_WARNING = 0,
  DEBUG_ERROR = 1,
};

void DebugPutc(unsigned char c);
void DebugClrScr(const uint8_t c);
void DebugPuts(char *str);
int DebugPrintf(const char *str, ...);
unsigned DebugSetColor(const unsigned c);
void DebugGotoXY(unsigned x, unsigned y);
void DebugGetXY(unsigned *x, unsigned *y);
int DebugGetHorz();
int DebugGetVert();

void debug_print(enum debug_level level, const char *str, ...);

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

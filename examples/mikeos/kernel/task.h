
#ifndef _TASK_H
#define _TASK_H
//****************************************************************************
//**
//**    task.h
//**		-Task manager
//**
//****************************************************************************
//============================================================================
//    INTERFACE REQUIRED HEADERS
//============================================================================

#include <stdint.h>
#include "mmngr_virtual.h"

//============================================================================
//    INTERFACE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================

/*
0x00000000-0x00400000 � Kernel reserved
0x00400000-0x80000000 � User land
0x80000000-0xffffffff � Kernel reserved
*/
#define KE_USER_START 0x00400000
#define KE_KERNEL_START 0x80000000

#define MAX_THREAD 5

//============================================================================
//    INTERFACE CLASS PROTOTYPES / EXTERNAL CLASS REFERENCES
//============================================================================
//============================================================================
//    INTERFACE STRUCTURES / UTILITY CLASSES
//============================================================================

/* be very careful with modifying this.
Reference tss.cpp ISR. */
typedef struct _trapFrame
{
	/* pushed by isr. */
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;
	/* pushed by pushf. */
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t esp;
	uint32_t ebp;
	/* pushed by cpu. */
	uint32_t eip;
	uint32_t cs;
	uint32_t flags;
	/* used only when coming from/to user mode. */
	//	uint32_t user_stack;
	//	uint32_t user_ss;
} trapFrame;

typedef unsigned int ktime_t;

#define THREAD_RUN 1
#define THREAD_BLOCK_SLEEP 2
/* we may have multiple block state flags. */
#define THREAD_BLOCK_STATE THREAD_BLOCK_SLEEP

struct _process;
typedef struct _thread
{
	uint32_t esp;
	uint32_t ss;
	uint32_t kernelEsp;
	uint32_t kernelSs;
	struct _process *parent;
	uint32_t priority;
	int state;
	ktime_t sleepTimeDelta;
} thread;

typedef struct _process
{
	int id;
	int priority;
	pdirectory *pageDirectory;
	int state;
	uint32_t imageBase;
	uint32_t imageSize;
	int threadCount;
	struct _thread threads[MAX_THREAD];
} process;

//============================================================================
//    INTERFACE DATA DECLARATIONS
//============================================================================
//============================================================================
//    INTERFACE FUNCTION PROTOTYPES
//============================================================================

void scheduler_initialize(void);
thread thread_create(void (*entry)(void), uint32_t esp, bool is_kernel);
void execute_idle();
bool queue_insert(thread t);
void *create_kernel_stack();
void thread_sleep(uint32_t ms);

//============================================================================
//    INTERFACE OBJECT CLASS DEFINITIONS
//============================================================================
//============================================================================
//    INTERFACE TRAILING HEADERS
//============================================================================
//****************************************************************************
//**
//**    END [task.h]
//**
//****************************************************************************

#endif

//****************************************************************************
//**
//**    task.cpp
//**		-Task Manager
//**
//****************************************************************************
//============================================================================
//    IMPLEMENTATION HEADERS
//============================================================================

#include "Include/string.h"
#include "Hal/Hal.h"
#include "FileSystem/fsys.h"
#include "image.h"
#include "mmngr_virtual.h"
#include "task.h"
#include "DebugDisplay.h"

//============================================================================
//    IMPLEMENTATION PRIVATE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

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

#define PROC_MAX 10
#define THREAD_MAX 10
#define PROC_INVALID_ID -1
#define TIME_QUANTUM 1

/* we limit number of processes and threads since we dont
have a proper heap allocator. This should be dynamically
allocated from non-paged pool. Note MAX_THREAD is different;
MAX_THREAD is per process, THREAD_MAX is max threads allowed
in system. */

thread _readyQueue[THREAD_MAX];
int _queue_last, _queue_first;
thread _idleThread;
thread *_currentTask;
thread _currentThreadLocal;
process _processList[PROC_MAX];

//============================================================================
//    INTERFACE DATA
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTION PROTOTYPES
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTIONS
//============================================================================

void switch_task(thread *, uint32_t);
void execute_thread(uint32_t);

/*** PROCESS LIST ***********************************/

/* add process to list. */
process *add_process(process p)
{
	for (int c = 0; c < PROC_MAX; c++)
	{
		/* id's of -1 are free. */
		if (_processList[c].id != PROC_INVALID_ID)
		{
			_processList[c] = p;
			return &_processList[c];
		}
	}
	return 0;
}

/* remove process from list. */
void remove_process(process p)
{
	for (int c = 0; c < PROC_MAX; c++)
	{
		if (_processList[c].id == p.id)
		{
			/* id's of -1 are free. */
			_processList[c].id = PROC_INVALID_ID;
			return;
		}
	}
}

/* init process list. */
void init_process_list()
{
	for (int c = 0; c < PROC_MAX; c++)
		_processList[c].id = PROC_INVALID_ID;
}

/*** READY QUEUE ************************************/

/* clear queue. */
void clear_queue()
{
	_queue_first = 0;
	_queue_last = 0;
}

/* insert thread. */
bool queue_insert(thread t)
{
	_readyQueue[_queue_last % THREAD_MAX] = t;
	_queue_last++;
	return true;
}

/* remove thread. */
thread queue_remove()
{
	thread t;
	t = _readyQueue[_queue_first % THREAD_MAX];
	_queue_first++;
	return t;
}

/* get top of queue. */
thread queue_get()
{
	return _readyQueue[_queue_first % THREAD_MAX];
}

/*** SCHEDULER *****************************************/

void scheduler_isr();
void (*old_isr)();
void idle_task();

/* schedule new task to run. */
void schedule()
{

	/* force a task switch. */
	__asm__ __volatile__("int $33");
}

/* set thread state flags. */
void thread_set_state(thread *t, uint32_t flags)
{

	/* set flags. */
	t->state |= flags;
}

/* remove thread state flags. */
void thread_remove_state(thread *t, uint32_t flags)
{

	/* remove flags. */
	t->state &= ~flags;
}

/* PUBLIC definition. */
void thread_sleep(uint32_t ms)
{

	/* go to sleep. */
	thread_set_state(_currentTask, THREAD_BLOCK_SLEEP);
	_currentTask->sleepTimeDelta = ms;
	schedule();
}

/* PUBLIC definition. */
void thread_wake()
{

	/* wake up. */
	thread_remove_state(_currentTask, THREAD_BLOCK_SLEEP);
	_currentTask->sleepTimeDelta = 0;
}

/* executes thread. */
void thread_execute(thread t)
{
	// execute_thread(t.esp);
	__asm__ __volatile__("mov %0, %%esp \n"
											 "pop %%gs			\n"
											 "pop %%fs			\n"
											 "pop %%es			\n"
											 "pop %%ds			\n"
											 "popal					\n"
											 "iret					\n" ::"r"(t.esp));
}

/*
	Note - the kernel mode stack should be allocated from non-paged pool.
	We have not covered memory allocators yet, so we are limited in what we
	can do. What we decided on was to reserve kernel stack sizes to 4K per
	thread, base at 0xe0000000 in the address space.
*/

/* create a new kernel space stack. */
int _kernel_stack_index = 0;
void *create_kernel_stack()
{

	physical_addr p;
	virtual_addr location;
	void *ret;

	/* we are reserving this area for 4k kernel stacks. */
#define KERNEL_STACK_ALLOC_BASE 0xe0000000

	/* allocate a 4k frame for the stack. */
	p = (physical_addr)pmmngr_alloc_block();
	if (!p)
		return 0;

	/* next free 4k memory block. */
	location = KERNEL_STACK_ALLOC_BASE + _kernel_stack_index * PAGE_SIZE;

	/* map it into kernel space. */
	vmmngr_mapPhysicalAddress(vmmngr_get_directory(), location, p, 3);

	/* we are returning top of stack. */
	ret = (void *)(location + PAGE_SIZE);

	/* prepare to allocate next 4k if we get called again. */
	_kernel_stack_index++;

	/* and return top of stack. */
	return ret;
}

/* creates thread. */
thread thread_create(void (*entry)(void), uint32_t esp, bool is_kernel)
{

	trapFrame *frame;
	thread t;

	/* for chapter 25, this paramater is ignored. */
	is_kernel = is_kernel;

	/* kernel and user selectors. */
#define USER_DATA 0x23
#define USER_CODE 0x1b
#define KERNEL_DATA 0x10
#define KERNEL_CODE 8

	/* adjust stack. We are about to push data on it. */
	esp -= sizeof(trapFrame);

	/* initialize task frame. */
	frame = ((trapFrame *)esp);
	frame->flags = 0x202;
	frame->eip = (uint32_t)entry;
	frame->ebp = 0;
	frame->esp = 0;
	frame->edi = 0;
	frame->esi = 0;
	frame->edx = 0;
	frame->ecx = 0;
	frame->ebx = 0;
	frame->eax = 0;

	/* set up segment selectors. */
	frame->cs = KERNEL_CODE;
	frame->ds = KERNEL_DATA;
	frame->es = KERNEL_DATA;
	frame->fs = KERNEL_DATA;
	frame->gs = KERNEL_DATA;
	t.ss = KERNEL_DATA;

	/* set stack. */
	t.esp = esp;

	/* ignore other fields. */
	t.parent = 0;
	t.priority = 0;
	t.state = THREAD_RUN;
	t.sleepTimeDelta = 0;
	return t;
}

/* execute idle thread. */
void execute_idle()
{

	/* just run idle thread. */
	thread_execute(_idleThread);
}

/* initialize scheduler. */
void scheduler_initialize(void)
{

	/* clear ready queue. */
	clear_queue();

	/* clear process list. */
	init_process_list();

	/* create idle thread and add it. */
	_idleThread = thread_create(idle_task, (uint32_t)create_kernel_stack(), true);

	/* set current thread to idle task and add it. */
	_currentThreadLocal = _idleThread;
	_currentTask = &_currentThreadLocal;
	queue_insert(_idleThread);

	/* register isr */
	// old_isr = getvect(32);
	// setvect_flags(32, scheduler_isr, 0x80);
	register_interrupt_handler(32, scheduler_isr);
}

/* schedule next task. */
void dispatch()
{

	/* We do Round Robin here, just remove and insert. */
next_thread:
	queue_remove();
	queue_insert(_currentThreadLocal);
	_currentThreadLocal = queue_get();

	/* make sure this thread is not blocked. */
	if (_currentThreadLocal.state & THREAD_BLOCK_STATE)
	{

		/* adjust time delta. */
		if (_currentThreadLocal.sleepTimeDelta > 0)
			_currentThreadLocal.sleepTimeDelta--;

		/* should we wake thread? */
		if (_currentThreadLocal.sleepTimeDelta == 0)
		{
			thread_wake();
			return;
		}

		/* not yet, go to next thread. */
		goto next_thread;
	}
}

/* gets called for each clock tick. */
void scheduler_tick()
{

	/* just run dispatcher. */
	dispatch();
}

/* Timer ISR called by PIT. */
void scheduler_isr()
{
	switch_task(_currentTask, old_isr);
}

/* idle thread. */
void idle_task()
{

	/* loop forever and yield to cpu. */
	while (1)
		__asm__ __volatile__("pause");
}

/*** IMAGE LOADING *********************************/

/* checks to make sure image is valid. */
bool validate_image(void *image)
{

	IMAGE_DOS_HEADER *dosHeader = 0;
	IMAGE_NT_HEADERS *ntHeaders = 0;

	/* validate program file */
	dosHeader = (IMAGE_DOS_HEADER *)image;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return false;
	if (dosHeader->e_lfanew == 0)
		return false;

	/* make sure header is valid */
	ntHeaders = (IMAGE_NT_HEADERS *)(dosHeader->e_lfanew + (uint32_t)image);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
		return false;

	/* only supporting for i386 archs */
	if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
		return false;

	/* only support 32 bit executable images */
	if (!(ntHeaders->FileHeader.Characteristics &
				(IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE)))
	{
		return false;
	}

	/* only support 32 bit optional header format */
	if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		return false;
	return true;
}

/* load program file into memory. */
bool load_image(char *appname, pdirectory *space, virtual_addr *image_base, uint32_t *image_size, virtual_addr *entry)
{

	unsigned char buf[512];
	IMAGE_DOS_HEADER *dosHeader;
	IMAGE_NT_HEADERS *ntHeaders;
	FILE file;
	unsigned int i;
	unsigned char *memory;

	/* open file */
	file = volOpenFile(appname);
	if (file.flags == FS_INVALID)
		return false;
	if ((file.flags & FS_DIRECTORY) == FS_DIRECTORY)
		return false;

	/* read 512 bytes into buffer */
	volReadFile(&file, buf, 512);
	if (!validate_image(buf))
	{
		volCloseFile(&file);
		return false;
	}
	dosHeader = (IMAGE_DOS_HEADER *)buf;
	ntHeaders = (IMAGE_NT_HEADERS *)(dosHeader->e_lfanew + (uint32_t)buf);

	/* allocate frame for the block we are about to read. */
	memory = (unsigned char *)pmmngr_alloc_block();

	/* copy the 512 bytes we just read and rest of page. */
	memcpy(memory, buf, 512);
	for (i = 1; i <= ntHeaders->OptionalHeader.SizeOfImage / 512; i++)
	{
		if (file.eof == 1)
			break;
		volReadFile(&file, memory + 512 * i, 512);
	}

	/* map first 4k block. */
	vmmngr_mapPhysicalAddress(space, ntHeaders->OptionalHeader.ImageBase,
														(uint32_t)memory, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);

	/* load rest of image */
	i = 1;
	while (file.eof != 1)
	{

		/* allocate new frame */
		unsigned char *cur = (unsigned char *)pmmngr_alloc_block();

		/* read block */
		int curBlock = 0;
		for (curBlock = 0; curBlock < 8; curBlock++)
		{
			if (file.eof == 1)
				break;
			volReadFile(&file, cur + 512 * curBlock, 512);
		}

		/* map page into process address space */
		vmmngr_mapPhysicalAddress(space, ntHeaders->OptionalHeader.ImageBase + i * 4096,
															(uint32_t)cur, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
		i++;
	}

	/* output paramaters. */
	if (image_base)
		*image_base = ntHeaders->OptionalHeader.ImageBase;
	if (image_size)
		*image_size = ntHeaders->OptionalHeader.SizeOfImage;
	if (entry)
		*entry = ntHeaders->OptionalHeader.AddressOfEntryPoint;

	/* close file. */
	volCloseFile(&file);
	return true;
}

/* clones kernel space into new address space. */
void clone_kernel_space(pdirectory *out)
{

	/* get current process directory. */
	pdirectory *proc = vmmngr_get_directory();

	/* copy kernel page tables into this new page directory.
	Recall that KERNEL SPACE is 0xc0000000, which starts at
	entry 768. */
	memcpy(&out->m_entries[768], &proc->m_entries[768], 256 * sizeof(pd_entry));
}

/* create new address space. */
pdirectory *create_address_space(void)
{

	pdirectory *space;

	/* allocate from PFN Bitmap. */
	space = (pdirectory *)pmmngr_alloc_block();

	/* clear page directory and clone kernel space. */
	vmmngr_pdirectory_clear(space);
	clone_kernel_space(space);
	return space;
}

/* creates user stack for main thread. */
bool create_user_stack(pdirectory *space)
{

	/* this will call our address space allocator
	to reserve area in user space. Until then,
	return error. */

	return false;
}

/* create process. */
bool create_process(char *appname)
{

	pdirectory *addressSpace;
	process pcb;
	virtual_addr base;
	virtual_addr entry;
	uint32_t size;

	/* create new address space. */
	addressSpace = create_address_space();

	/* try to load image into it. */
	if (!load_image(appname, addressSpace, &base, &size, &entry))
		return false;

	/* create stack space for main thread. */
	if (!create_user_stack(addressSpace))
		return false;

	/* create process. */
	pcb.imageBase = 0;
	pcb.pageDirectory = addressSpace;

	/* create main thread. */
	pcb.threadCount = 0;

	/* add process. */
	pcb.threads[0].parent = add_process(pcb);

	/* schedule thread to run. */
	queue_insert(pcb.threads[0]);
	return true;
}

//============================================================================
//    INTERFACE CLASS BODIES
//============================================================================
//****************************************************************************
//**
//**    END[task.cpp]
//**
//****************************************************************************

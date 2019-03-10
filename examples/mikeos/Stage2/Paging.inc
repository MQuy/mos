
;*******************************************************
;
;	Paging.inc
;		Basic paging for bootloader
;
;	OS Development Series
;*******************************************************

; Added sence Tutorial 17

%ifndef __PAGING_INC_67343546FDCC56AAB872_INCLUDED__
%define __PAGING_INC_67343546FDCC56AAB872_INCLUDED__

bits	32

; page directory table
%define		PAGE_DIR			0x9C000

; 0th page table. Address must be 4KB aligned
%define		PAGE_TABLE_0		0x9D000

; 768th page table. Address must be 4KB aligned
%define		PAGE_TABLE_768		0x9E000

; each page table has 1024 entries
%define		PAGE_TABLE_ENTRIES	1024

; attributes (page is present;page is writable; supervisor mode)
%define		PRIV				3

;****************************************
;	Enable Paging
;****************************************

EnablePaging:
	pusha										; save stack frame

	;------------------------------------------
	;	idenitity map 1st page table (4MB)
	;------------------------------------------

	mov		eax, PAGE_TABLE_0					; first page table
	mov		ebx, 0x0 | PRIV						; starting physical address of page
	mov		ecx, PAGE_TABLE_ENTRIES				; for every page in table...
.loop:
	mov		dword [eax], ebx					; write the entry
	add		eax, 4								; go to next page entry in table (Each entry is 4 bytes)
	add		ebx, 4096							; go to next page address (Each page is 4Kb)
	loop	.loop								; go to next entry

	;------------------------------------------
	;	set up the entries in the directory table
	;------------------------------------------

	mov		eax, PAGE_TABLE_0 | PRIV			; 1st table is directory entry 0
	mov		dword [PAGE_DIR], eax

	mov		eax, PAGE_TABLE_768 | PRIV			; 768th entry in directory table
	mov		dword [PAGE_DIR+(768*4)], eax

	;------------------------------------------
	;	install directory table
	;------------------------------------------

	mov		eax, PAGE_DIR
	mov		cr3, eax

	;------------------------------------------
	;	enable paging
	;------------------------------------------

	mov		eax, cr0
	or		eax, 0x80000000
	mov		cr0, eax

	;------------------------------------------
	;	map the 768th table to physical addr 1MB
	;	the 768th table starts the 3gb virtual address
	;------------------------------------------
 
	mov		eax, PAGE_TABLE_768				; first page table
	mov		ebx, 0x100000 | PRIV			; starting physical address of page
	mov		ecx, PAGE_TABLE_ENTRIES			; for every page in table...
.loop2:
	mov		dword [eax], ebx				; write the entry
	add		eax, 4							; go to next page entry in table (Each entry is 4 bytes)
	add		ebx, 4096						; go to next page address (Each page is 4Kb)
	loop	.loop2							; go to next entry

	popa
	ret

%endif

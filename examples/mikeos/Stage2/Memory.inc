
;*************************************************
;	Memory.inc
;		-Basic memory routines
;
;	OS Development Series
;*************************************************

%ifndef __MEMORY_INC_67343546FDCC56AAB872_INCLUDED__
%define __MEMORY_INC_67343546FDCC56AAB872_INCLUDED__

bits 16

;--------------------------------------------
;	Memory map entry structure
;--------------------------------------------

struc	MemoryMapEntry
	.baseAddress		resq	1
	.length				resq	1
	.type				resd	1
	.acpi_null			resd	1
endstruc

;---------------------------------------------
;	Get memory map from bios
;	/in es:di->destination buffer for entries
;	/ret bp=entry count
;---------------------------------------------

BiosGetMemoryMap:

	xor		ebx, ebx
	xor		bp, bp
	mov		edx, 0x0534D4150		; Place "SMAP" into edx
	mov		eax, 0xe820
	mov		[es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov		ecx, 24
	int		0x15
	jc		short .failed			; carry set on first call means "unsupported function"
	mov		edx, 0x0534D4150		; Some BIOSes apparently trash this register?
	cmp		eax, edx				; on success, eax must have been reset to "SMAP"
	jne		short .failed
	test	ebx, ebx				; ebx = 0 implies list is only 1 entry long (worthless)
	je		short .failed
	jmp		short .jmpin
.e820lp:
	mov		eax, 0xe820				; eax, ecx get trashed on every int 0x15 call
	mov		[es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov		ecx, 24					; ask for 24 bytes again
	int		0x15
	jc		short .e820f			; carry set means "end of list already reached"
	mov		edx, 0x0534D4150		; repair potentially trashed register
.jmpin:
	jcxz	.skipent				; skip any 0 length entries
	cmp		cl, 20					; got a 24 byte ACPI 3.X response?
	jbe		short .notext
	test	byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je		short .skipent
.notext:
	mov		ecx, [es:di + 8]		; get lower dword of memory region length
	test	ecx, ecx				; is the qword == 0?
	jne		short .goodent
	mov		ecx, [es:di + 12]		; get upper dword of memory region length
	jecxz	.skipent				; if length qword is 0, skip entry
.goodent:
	inc		bp						; got a good entry: ++count, move to next storage spot
	add		di, 24
.skipent:
	test	ebx, ebx				; if ebx resets to 0, list is complete
	jne		short .e820lp
.e820f:
	ret								; bp=entry count
.failed:
	stc								; "function unsupported" error exit
	ret

;---------------------------------------------
;	Get memory size for >64M configuations (32 bit)
;	ret\ ax=KB between 1MB and 16MB
;	ret\ bx=number of 64K blocks above 16MB
;	ret\ bx=0 and ax= -1 on error
;---------------------------------------------

BiosGetMemorySize64MB_32bit:
	push	ecx
	push	edx
	xor		ecx, ecx
	xor		edx, edx
	mov		eax, 0xe881
	int		0x15
	jc		.error
	jcxz	.use_ax				;bios may have stored it in ax,bx or cx,dx. test if cx is 0
	mov		eax, ecx			;its not, so it should contain mem size; store it
	mov		ebx, edx

.use_ax:
	pop		edx				;mem size is in ax and bx already, return it
	pop		ecx
	ret

.error:
	mov		ax, -1
	mov		bx, 0
	pop		edx
	pop		ecx
	ret

;---------------------------------------------
;	Get memory size for >64M configuations
;	ret\ ax=KB between 1MB and 16MB
;	ret\ bx=number of 64K blocks above 16MB
;	ret\ bx=0 and ax= -1 on error
;---------------------------------------------

BiosGetMemorySize64MB:
	push	ecx
	push	edx
	xor		ecx, ecx
	xor		edx, edx
	mov		ax, 0xe801
	int		0x15	
	jc		.error
	cmp		ah, 0x86		;unsupported function
	je		.error
	cmp		ah, 0x80		;invalid command
	je		.error
	jcxz	.use_ax			;bios may have stored it in ax,bx or cx,dx. test if cx is 0
	mov		ax, cx			;its not, so it should contain mem size; store it
	mov		bx, dx

.use_ax:
	pop		edx				;mem size is in ax and bx already, return it
	pop		ecx
	ret

.error:
	mov		ax, -1
	mov		bx, 0
	pop		edx
	pop		ecx
	ret

;---------------------------------------------
;	Get amount of contiguous KB from addr 0
;	ret\ ax=KB size from address 0
;---------------------------------------------

BiosGetMemorySize:
	int	0x12
	ret

;---------------------------------------------
;	Get contiguous exetended memory size
;	ret\ ax=KB size above 1MB; ax= -1 on error
;---------------------------------------------

BiosGetExtendedMemorySize:
	mov		ax, 0x88
	int		0x15
	jc		.error
	test	ax, ax		; if size=0
	je		.error
	cmp		ah, 0x86	;unsupported function
	je		.error
	cmp		ah, 0x80	;invalid command
	je		.error
	ret
.error:
	mov		ax, -1
	ret


%endif

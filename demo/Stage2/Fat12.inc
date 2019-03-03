
;*******************************************************
;
;	Fat12.inc
;		FAT12 filesystem for 3-1/2 floppies
;
;	OS Development Series
;*******************************************************

%ifndef __FAT12_INC_67343546FDCC56AAB872_INCLUDED__
%define __FAT12_INC_67343546FDCC56AAB872_INCLUDED__

bits	16

%include "Floppy16.inc"					; the erm.. floppy driver

%define ROOT_OFFSET 0x2e00
%define FAT_SEG 0x2c0
%define ROOT_SEG 0x2e0

;*******************************************
; LoadRoot ()
;	- Load Root Directory Table to 0x7e00
;*******************************************

LoadRoot:

	pusha							; store registers
	push	es

     ; compute size of root directory and store in "cx"
     
	xor     cx, cx						; clear registers
 	xor     dx, dx
	mov     ax, 32					; 32 byte directory entry
	mul     WORD [bpbRootEntries]				; total size of directory
	div     WORD [bpbBytesPerSector]			; sectors used by directory
	xchg    ax, cx						; move into AX

     ; compute location of root directory and store in "ax"
     
	mov     al, BYTE [bpbNumberOfFATs]			; number of FATs
	mul     WORD [bpbSectorsPerFAT]				; sectors used by FATs
	add     ax, WORD [bpbReservedSectors]
	mov     WORD [datasector], ax				; base of root directory
	add     WORD [datasector], cx

     ; read root directory into 0x7e00
 
 	push	word ROOT_SEG
	pop		es
	mov     bx, 0								; copy root dir
	call    ReadSectors							; read in directory table
	pop		es
	popa										; restore registers and return
	ret

;*******************************************
; LoadFAT ()
;	- Loads FAT table to 0x7c00
;
;	Parm/ ES:DI => Root Directory Table
;*******************************************

LoadFAT:

	pusha							; store registers
	push	es

     ; compute size of FAT and store in "cx"
     
	xor     ax, ax
	mov     al, BYTE [bpbNumberOfFATs]			; number of FATs
	mul     WORD [bpbSectorsPerFAT]				; sectors used by FATs
	mov     cx, ax

     ; compute location of FAT and store in "ax"

	mov     ax, WORD [bpbReservedSectors]

     ; read FAT into memory (Overwrite our bootloader at 0x7c00)

	push	word FAT_SEG
	pop		es
	xor		bx, bx
	call    ReadSectors
	pop		es
	popa							; restore registers and return
	ret
	
;*******************************************
; FindFile ()
;	- Search for filename in root table
;
; parm/ DS:SI => File name
; ret/ AX => File index number in directory table. -1 if error
;*******************************************

FindFile:

	push	cx						; store registers
	push	dx
	push	bx
	mov	bx, si						; copy filename for later

     ; browse root directory for binary image

	mov     cx, WORD [bpbRootEntries]			; load loop counter
	mov     di, ROOT_OFFSET						; locate first root entry at 1 MB mark
	cld							; clear direction flag

.LOOP:
	push    cx
	mov     cx, 11					; eleven character name. Image name is in SI
	mov	si, bx						; image name is in BX
 	push    di
     rep  cmpsb							; test for entry match
	pop     di
	je      .Found
	pop     cx
	add     di, 32					; queue next directory entry
	loop    .LOOP

.NotFound:
	pop	bx						; restore registers and return
	pop	dx
	pop	cx
	mov	ax, -1						; set error code
	ret

.Found:
	pop	ax						; return value into AX contains entry of file
	pop	bx						; restore registers and return
	pop	dx
	pop	cx
	ret

;*******************************************
; LoadFile ()
;	- Load file
; parm/ ES:SI => File to load
; parm/ EBX:BP => Buffer to load file to
; ret/ AX => -1 on error, 0 on success
;*******************************************

LoadFile:

	xor		ecx, ecx					; size of file in sectors
	push	ecx

.FIND_FILE:

	push	bx						; BX=>BP points to buffer to write to; store it for later
	push	bp
	call	FindFile				; find our file. ES:SI contains our filename
	cmp		ax, -1
	jne		.LOAD_IMAGE_PRE
	pop		bp
	pop		bx
	pop		ecx
	mov		ax, -1
	ret

.LOAD_IMAGE_PRE:

	sub		edi, ROOT_OFFSET
	sub		eax, ROOT_OFFSET

	; get starting cluster

	push	word ROOT_SEG				;root segment loc
	pop		es
	mov		dx, WORD [es:di + 0x001A]	; DI points to file entry in root directory table. Refrence the table...
	mov		WORD [cluster], dx			; file's first cluster
	pop		bx							; get location to write to so we dont screw up the stack
	pop		es
	push    bx							; store location for later again
	push	es
	call	LoadFAT

.LOAD_IMAGE:

	; load the cluster

	mov		ax, WORD [cluster]		; cluster to read
	pop		es						; bx:bp=es:bx
	pop		bx
	call	ClusterLBA
	xor		cx, cx
	mov     cl, BYTE [bpbSectorsPerCluster]

	call	ReadSectors

	pop		ecx
	inc		ecx
	push	ecx

	push	bx
	push	es

	mov		ax, FAT_SEG						;start reading from fat
	mov		es, ax
	xor		bx, bx

	; get next cluster

	mov     ax, WORD [cluster]				; identify current cluster
	mov     cx, ax							; copy current cluster
	mov     dx, ax							; copy current cluster
	shr     dx, 0x0001						; divide by two
	add     cx, dx							; sum for (3/2)

	mov		bx, 0							;location of fat in memory
	add		bx, cx
	mov		dx, WORD [es:bx]
	test	ax, 0x0001						; test for odd or even cluster
	jnz		.ODD_CLUSTER

.EVEN_CLUSTER:

	and		dx, 0000111111111111b	; take low 12 bits
	jmp		.DONE

.ODD_CLUSTER:

	shr		dx, 0x0004				; take high 12 bits

.DONE:

	mov		WORD [cluster], dx
	cmp		dx, 0x0ff0				; test for end of file marker
	jb		.LOAD_IMAGE

.SUCCESS:
	pop		es
	pop		bx
	pop		ecx
	xor		ax, ax
	ret

%endif		;__FAT12_INC_67343546FDCC56AAB872_INCLUDED__






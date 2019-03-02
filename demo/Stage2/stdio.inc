
;*************************************************
;	stdio.inc
;		-Input/Output routines
;
;	OS Development Series
;*************************************************

%ifndef __STDIO_INC_67343546FDCC56AAB872_INCLUDED__
%define __STDIO_INC_67343546FDCC56AAB872_INCLUDED__


;==========================================================
;
;	 16 Bit Real Mode Routines
;==========================================================


;************************************************;
;	Puts16 ()
;		-Prints a null terminated string
;	DS=>SI: 0 terminated string
;************************************************;

bits	16

Puts16:
		pusha				; save registers
.Loop1:
		lodsb				; load next byte from string from SI to AL
		or	al, al			; Does AL=0?
		jz	Puts16Done		; Yep, null terminator found-bail out
		mov	ah, 0eh			; Nope-Print the character
		int	10h			; invoke BIOS
		jmp	.Loop1			; Repeat until null terminator found
Puts16Done:
		popa				; restore registers
		ret				; we are done, so return


;==========================================================
;
;	 32 Bit Protected Mode Routines
;==========================================================

bits 32

%define		VIDMEM	0xB8000			; video memory
%define		COLS	80			; width and height of screen
%define		LINES	25
%define		CHAR_ATTRIB 63			; character attribute (White text on light blue background)

_CurX db 0					; current x/y location
_CurY db 0

;**************************************************;
;	Putch32 ()
;		- Prints a character to screen
;	BL => Character to print
;**************************************************;

Putch32:

	pusha				; save registers
	mov	edi, VIDMEM		; get pointer to video memory

	;-------------------------------;
	;   Get current position	;
	;-------------------------------;

	xor	eax, eax		; clear eax

		;--------------------------------
		; Remember: currentPos = x + y * COLS! x and y are in _CurX and _CurY.
		; Because there are two bytes per character, COLS=number of characters in a line.
		; We have to multiply this by 2 to get number of bytes per line. This is the screen width,
		; so multiply screen with * _CurY to get current line
		;--------------------------------

		mov	ecx, COLS*2		; Mode 7 has 2 bytes per char, so its COLS*2 bytes per line
		mov	al, byte [_CurY]	; get y pos
		mul	ecx			; multiply y*COLS
		push	eax			; save eax--the multiplication

		;--------------------------------
		; Now y * screen width is in eax. Now, just add _CurX. But, again remember that _CurX is relative
		; to the current character count, not byte count. Because there are two bytes per character, we
		; have to multiply _CurX by 2 first, then add it to our screen width * y.
		;--------------------------------

		mov	al, byte [_CurX]	; multiply _CurX by 2 because it is 2 bytes per char
		mov	cl, 2
		mul	cl
		pop	ecx			; pop y*COLS result
		add	eax, ecx

		;-------------------------------
		; Now eax contains the offset address to draw the character at, so just add it to the base address
		; of video memory (Stored in edi)
		;-------------------------------

		xor	ecx, ecx
		add	edi, eax		; add it to the base address

	;-------------------------------;
	;   Watch for new line          ;
	;-------------------------------;

	cmp	bl, 0x0A		; is it a newline character?
	je	.Row			; yep--go to next row

	;-------------------------------;
	;   Print a character           ;
	;-------------------------------;

	mov	dl, bl			; Get character
	mov	dh, CHAR_ATTRIB		; the character attribute
	mov	word [edi], dx		; write to video display

	;-------------------------------;
	;   Update next position        ;
	;-------------------------------;

	inc	byte [_CurX]		; go to next character
;	cmp	byte [_CurX], COLS		; are we at the end of the line?
;	je	.Row			; yep-go to next row
	jmp	.done			; nope, bail out

	;-------------------------------;
	;   Go to next row              ;
	;-------------------------------;

.Row:
	mov	byte [_CurX], 0		; go back to col 0
	inc	byte [_CurY]		; go to next row

	;-------------------------------;
	;   Restore registers & return  ;
	;-------------------------------;

.done:
	popa				; restore registers and return
	ret

;**************************************************;
;	Puts32 ()
;		- Prints a null terminated string
;	parm\ EBX = address of string to print
;**************************************************;

Puts32:

	;-------------------------------;
	;   Store registers             ;
	;-------------------------------;

	pusha				; save registers
	push	ebx			; copy the string address
	pop	edi

.loop:

	;-------------------------------;
	;   Get character               ;
	;-------------------------------;

	mov	bl, byte [edi]		; get next character
	cmp	bl, 0			; is it 0 (Null terminator)?
	je	.done			; yep-bail out

	;-------------------------------;
	;   Print the character         ;
	;-------------------------------;

	call	Putch32			; Nope-print it out

	;-------------------------------;
	;   Go to next character        ;
	;-------------------------------;

	inc	edi			; go to next character
	jmp	.loop

.done:

	;-------------------------------;
	;   Update hardware cursor      ;
	;-------------------------------;

	; Its more efficiant to update the cursor after displaying
	; the complete string because direct VGA is slow

	mov	bh, byte [_CurY]	; get current position
	mov	bl, byte [_CurX]
	call	MovCur			; update cursor

	popa				; restore registers, and return
	ret

;**************************************************;
;	MoveCur ()
;		- Update hardware cursor
;	parm/ bh = Y pos
;	parm/ bl = x pos
;**************************************************;

bits 32

MovCur:

	pusha				; save registers (aren't you getting tired of this comment?)

	;-------------------------------;
	;   Get current position        ;
	;-------------------------------;

	; Here, _CurX and _CurY are relitave to the current position on screen, not in memory.
	; That is, we don't need to worry about the byte alignment we do when displaying characters,
	; so just follow the forumla: location = _CurX + _CurY * COLS

	xor	eax, eax
	mov	ecx, COLS
	mov	al, bh			; get y pos
	mul	ecx			; multiply y*COLS
	add	al, bl			; Now add x
	mov	ebx, eax

	;--------------------------------------;
	;   Set low byte index to VGA register ;
	;--------------------------------------;

	mov	al, 0x0f
	mov	dx, 0x03D4
	out	dx, al

	mov	al, bl
	mov	dx, 0x03D5
	out	dx, al			; low byte

	;---------------------------------------;
	;   Set high byte index to VGA register ;
	;---------------------------------------;

	xor	eax, eax

	mov	al, 0x0e
	mov	dx, 0x03D4
	out	dx, al

	mov	al, bh
	mov	dx, 0x03D5
	out	dx, al			; high byte

	popa
	ret

;**************************************************;
;	ClrScr32 ()
;		- Clears screen
;**************************************************;

bits 32

ClrScr32:

	pusha
	cld
	mov	edi, VIDMEM
	mov	cx, 2000
	mov	ah, CHAR_ATTRIB
	mov	al, ' '	
	rep	stosw

	mov	byte [_CurX], 0
	mov	byte [_CurY], 0
	popa
	ret

;**************************************************;
;	GotoXY ()
;		- Set current X/Y location
;	parm\	AL=X position
;	parm\	AH=Y position
;**************************************************;

bits 32

GotoXY:
	pusha
	mov	[_CurX], al		; just set the current position
	mov	[_CurY], ah
	popa
	ret




%endif ;__STDIO_INC_67343546FDCC56AAB872_INCLUDED__

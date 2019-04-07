global enter_usermode

enter_usermode:
	cli
	mov ax, 0x23	; user mode data selector is 0x20 (GDT entry 3). Also sets RPL to 3
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push 0x23		; SS, notice it uses same selector as above
	push esp		; ESP
	pushfd			; EFLAGS

	pop eax
	or eax, 0x200	; enable IF in EFLAGS
	push eax

	push 0x1b		; CS, user mode code selector is 0x18. With RPL 3 this is 0x1b
	lea eax, [a]	; EIP first
	push eax
	iretd

a:
	add esp, 4 
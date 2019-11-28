[global enter_usermode]
enter_usermode:
	cli

	mov ax,0x23
	mov ds,ax
	mov es,ax 
	mov fs,ax 
	mov gs,ax ;we don't need to worry about SS. it's handled by iret

	mov eax, [esp + 4]
	mov ebx, [esp + 8]
	push 0x23 ;user data segment with bottom 2 bits set for ring 3
	push eax ;push our current stack just for the heck of it
	pushf			; EFLAGS

	pop eax
	or eax, 0x200	; enable IF in EFLAGS
	push eax

	push 0x1B; ;user code segment with bottom 2 bits set for ring 3
	push ebx  ;may need to remove the _ for this to work right 
	iret
[global enter_usermode]
enter_usermode:
	cli

	mov ax,0x23
	mov ds,ax
	mov es,ax 
	mov fs,ax 
	mov gs,ax ;we don't need to worry about SS. it's handled by iret

	mov eax, [esp + 4]
	; set address in user stack which causes the page fault when finishing a user thread
	sub eax, 4
	mov ebx, [esp + 12]
	mov dword [eax], ebx

	mov ebx, [esp + 8] ; user entry

	push 0x23 ;user data segment with bottom 2 bits set for ring 3
	push eax ;push our current stack just for the heck of it
	pushf			; EFLAGS

	pop eax
	or eax, 0x200	; enable IF in EFLAGS
	push eax

	push 0x1B; ;user code segment with bottom 2 bits set for ring 3
	push ebx  ;may need to remove the _ for this to work right 
	iret

[global return_usermode]
return_usermode:
	cli

	mov ax,0x23
	mov ds,ax
	mov es,ax 
	mov fs,ax 
	mov gs,ax ;we don't need to worry about SS. it's handled by iret

	mov eax, [esp + 4]

	push dword [eax + 18*4] ;user data segment with bottom 2 bits set for ring 3
	push dword [eax + 17*4] ;push our current stack just for the heck of it
	push dword [eax + 16*4]	;EFLAGS
	push dword [eax + 15*4] ;segment selector
	push dword [eax + 14*4] ;eip

	mov edi, [eax + 4*4]
	mov esi, [eax + 5*4]
	mov ebp, [eax + 6*4]
	mov ebx, [eax + 8*4]
	mov edx, [eax + 9*4]
	mov ecx, [eax + 10*4]
	mov eax, [eax + 11*4]

	iret

[global sigjump_usermode]
sigjump_usermode:
	noop

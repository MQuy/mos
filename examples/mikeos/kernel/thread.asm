[extern scheduler_tick]
[extern tss_set_stack]

[global switch_task]
switch_task:
      ;
			; clear interrupts and save context.
			;
			cli
			pushad
			;
			; if no current task, just return.
			;
			mov eax, [edi]
			cmp eax, 0
			jz  interrupt_return
			;
			; save selectors.
			;
			push ds
			push es
			push fs
			push gs
			;
			; switch to kernel segments.
			;
			mov ax, 0x10
			mov ds, ax
			mov es, ax
			mov fs, ax
			mov gs, ax
			;
			; save esp.
			;
			mov eax, [edi]
			mov [eax], esp
			;
			; call scheduler.
			;
			call scheduler_tick
			;
			; restore esp.
			;
			mov eax, [edi]
			mov esp, [eax]
			;
			; Call tss_set_stack (kernelSS, kernelESP).
			; This code will be needed later for user tasks.
			;
			; push dword 0x8(eax)
			; push dword 0x12(eax)
			; call tss_set_stack
			; add esp, 8
			;
			; send EOI and restore context.
			;
			pop gs
			pop fs
			pop es
			pop ds
	interrupt_return:
			;
			; test if we need to call old ISR.
			;
			mov eax, esi
			cmp eax, 0
			jne chain_interrupt
			;
			; if old_isr is null, send EOI and return.
			;
			mov al,0x20
			out 0x20,al
			popa
			iret
			;
			; if old_isr is valid, jump to it. This calls
			; our PIT timer interrupt handler.
			;
	chain_interrupt:
			popa
			jmp esi

[global execute_thread]
execute_thread:
    mov esp, edi
		pop gs
		pop fs
		pop es
		pop ds
		popa
		iret
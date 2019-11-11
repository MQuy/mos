MULTIBOOT2_ARCH  EQU 0                                 ; 0 = x86/x86-64
MULTIBOOT2_LEN   EQU (multiboot2_header_start-multiboot2_header_end)
MULTIBOOT2_MAGIC EQU 0xe85250d6

section .multiboot align=4096
multiboot2_header_start:
	dd MULTIBOOT2_MAGIC                            ; Multiboot2 magic number
	dd MULTIBOOT2_ARCH                             ; Architecture
	dd MULTIBOOT2_LEN                              ; Multiboot header length
	dd 0x100000000 - MULTIBOOT2_LEN - MULTIBOOT2_ARCH - MULTIBOOT2_MAGIC

align 8
multiboot2_tag_fb_start:
	dw 5                                    ; framebuffer settings
	dw 1
	dd multiboot2_tag_fb_end - multiboot2_tag_fb_start
	dd 800
	dd 600
	dd 32
multiboot2_tag_fb_end:

align 8
multiboot2_tag_end_start:
	dw 0                                    ; last tag
	dw 0
	dd multiboot2_tag_end_end - multiboot2_tag_end_start
multiboot2_tag_end_end:

multiboot2_header_end:

section .bss
align 16
stack_bottom:
resb 16384 ; 16 KiB
stack_top:

section .text
global _start:function (_start.end - _start)
_start:
mov esp, stack_top

push ebx
push eax
[extern kernel_main]
call kernel_main

cli
.hang:	hlt
jmp .hang
_start.end:
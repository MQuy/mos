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


KERNEL_VIRTUAL_BASE equ 0xC0000000                  ; 3GB
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22)  ; Page directory index of kernel's 4MB PTE. 768
KERNEL_STACK_SIZE equ 0x4000 												; reserve initial kernel stack space -- that's 16k.

section .data
align 0x1000
boot_page_directory:
	dd 0x00000083
  times (KERNEL_PAGE_NUMBER - 1) dd 0                 ; Pages before kernel space.
  ; This page directory entry defines a 4MB page containing the kernel.
  dd 0x00000083
  times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0  ; Pages after the kernel image.

section .text

global _start
_start:

kernel_enable_paging:
	; NOTE: Until paging is set up, the code must be position-independent and use physical
	; addresses, not virtual ones!
	mov ecx, (boot_page_directory - KERNEL_VIRTUAL_BASE)
	mov cr3, ecx                                        ; Load Page Directory Base Register.

	mov ecx, cr4
	or ecx, 0x00000010                          ; Set PSE bit in CR4 to enable 4MB pages.
	mov cr4, ecx

	mov ecx, cr0
	or ecx, 0x80000000                          ; Set PG bit in CR0 to enable paging.
	mov cr0, ecx

	; Start fetching instructions in kernel space.
	; Since eip at this point holds the physical address of this command (approximately 0x00100000)
	; we need to do a long jump to the correct virtual address of StartInHigherHalf which is
	; approximately 0xC0100000.
	lea ecx, [start_in_higher_half]
	jmp ecx                                                     ; NOTE: Must be absolute jump!

extern kernel_main
start_in_higher_half:
	; Unmap the identity-mapped first 4MB of physical address space. It should not be needed
	; anymore.
	mov dword [boot_page_directory], 0
	invlpg [0]

	; NOTE: From now on, paging should be enabled. The first 4MB of physical address space is
	; mapped starting at KERNEL_VIRTUAL_BASE. Everything is linked to this address, so no more
	; position-independent code or funny business with virtual-to-physical address translation
	; should be necessary. We now have a higher-half kernel.

	mov esp, kernel_stack+KERNEL_STACK_SIZE           ; set up the stack

	push eax                           ; pass Multiboot magic number

	; pass Multiboot info structure -- WARNING: This is a physical address and may not be
	; in the first 4MB!
	add ebx, KERNEL_VIRTUAL_BASE ; make the address virtual
	push ebx ; push it on the stack for kernel_main()

	call  kernel_main                  ; call kernel proper
	hlt                          ; halt machine should kernel return


section .bss
align 16
kernel_stack:
	resb KERNEL_STACK_SIZE

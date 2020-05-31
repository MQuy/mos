ENTRY(_start)

SECTIONS
{
	kernel_boot = 0x00100000;
	kernel_higher_half = 0xc0000000;
	
	. = 0xC0100000;
	kernel_start = .;
 
	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text ALIGN(4096) : AT(ADDR(.text) - kernel_higher_half)
	{
		kernel_text_start = .;
		*(.multiboot)
		*(.text .text.*)
		kernel_text_end = .;
	}
 
	/* Read-only data. */
	.rodata ALIGN(4096) : AT(ADDR(.rodata) - kernel_higher_half)
	{
		*(.rodata .rodata.*)
	}
 
	/* Read-write data (initialized) */
	.data ALIGN(4096) : AT(ADDR(.data) - kernel_higher_half)
	{
		kernel_data_start = .;
		*(.data .data.*)
		*(.symbols)
		kernel_data_end = .;
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4096) : AT(ADDR(.bss) - kernel_higher_half)
	{
		*(COMMON)
		*(.bss .bss.*)
		*(.stack)
	}
 
 	.eh_frame ALIGN(4096) : AT(ADDR(.eh_frame) - kernel_higher_half)
	{
		*(.eh_frame)
	}

	kernel_end = .;

	/DISCARD/ :
	{
		*(.comment)
	}
}

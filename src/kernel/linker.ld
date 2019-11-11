ENTRY(_start)
 
SECTIONS
{
	. = 1M;
	kernel_start = .;
 
	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text .text.*)
	}
 
	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata .rodata.*)
	}
 
	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data .data.*)
		*(.symbols)
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss .bss.*)
		*(.stack)
	}
 
 	.eh_frame BLOCK(4K) : ALIGN(4K)
	{
		*(.eh_frame)
	}

	kernel_end = .;

	/DISCARD/ :
	{
		*(.comment)
	}
}
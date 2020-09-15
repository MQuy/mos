ENTRY(main)

SECTIONS
{
	. = 0x00100000;
 
	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text ALIGN(4096) : AT(ADDR(.text))
	{
		*(.text .text.*)
	}
 
	/* Read-only data. */
	.rodata ALIGN(4096) : AT(ADDR(.rodata))
	{
		*(.rodata .rodata.*)
	}
 
	/* Read-write data (initialized) */
	.data ALIGN(4096) : AT(ADDR(.data))
	{
		*(.data .data.*)
		*(.symbols)
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4096) : AT(ADDR(.bss))
	{
		*(COMMON)
		*(.bss .bss.*)
		*(.stack)
	}
 
 	.eh_frame ALIGN(4096) : AT(ADDR(.eh_frame))
	{
		*(.eh_frame)
	}

	/DISCARD/ :
	{
		*(.comment)
	}
}

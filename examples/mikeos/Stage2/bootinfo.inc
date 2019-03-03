
;*******************************************************
;
;	bootinfo.inc
;		multiboot information structure
;
;	OS Development Series
;*******************************************************

%ifndef __BOOTINFO_INC_67343546FDCC56AAB872_INCLUDED__
%define __BOOTINFO_INC_67343546FDCC56AAB872_INCLUDED__

struc multiboot_info
	.flags				resd	1
	.memoryLo			resd	1
	.memoryHi			resd	1
	.kernel_size resd 1
	.bootDevice			resd	1
	.cmdLine			resd	1
	.mods_count			resd	1
	.mods_addr			resd	1
	.syms0				resd	1
	.syms1				resd	1
	.syms2				resd	1
	.mmap_length		resd	1
	.mmap_addr			resd	1
	.drives_length		resd	1
	.drives_addr		resd	1
	.config_table		resd	1
	.bootloader_name	resd	1
	.apm_table			resd	1
	.vbe_control_info	resd	1
	.vbe_mode_info		resd	1
	.vbe_mode			resw	1
	.vbe_interface_seg	resw	1
	.vbe_interface_off	resw	1
	.vbe_interface_len	resw	1
endstruc


%endif

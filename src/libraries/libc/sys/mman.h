#ifndef LIBC_SYS_MMANH_H
#define LIBC_SYS_MMANH_H

#define PROT_READ 0x1			  /* page can be read */
#define PROT_WRITE 0x2			  /* page can be written */
#define PROT_EXEC 0x4			  /* page can be executed */
#define PROT_SEM 0x8			  /* page may be used for atomic ops */
#define PROT_NONE 0x0			  /* page can not be accessed */
#define PROT_GROWSDOWN 0x01000000 /* mprotect flag: extend change to start of growsdown vma */
#define PROT_GROWSUP 0x02000000	  /* mprotect flag: extend change to end of growsup vma */

#define MAP_SHARED 0x01	   /* Share changes */
#define MAP_PRIVATE 0x02   /* Changes are private */
#define MAP_TYPE 0x0f	   /* Mask for type of mapping */
#define MAP_FIXED 0x10	   /* Interpret addr exactly */
#define MAP_ANONYMOUS 0x20 /* don't use a file */

#endif

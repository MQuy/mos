#ifndef _LIBC_MEMCOPY_H
#define _LIBC_MEMCOPY_H 1

#undef OP_T_THRES
#define OP_T_THRES 8

#undef BYTE_COPY_FWD
#define BYTE_COPY_FWD(dst_bp, src_bp, nbytes)                                          \
	do                                                                                 \
	{                                                                                  \
		int __d0;                                                                      \
		asm volatile(		 /* Clear the direction flag, so copying goes forward.  */ \
					 "cld\n" /* Copy bytes.  */                                        \
					 "rep\n"                                                           \
					 "movsb"                                                           \
					 : "=D"(dst_bp), "=S"(src_bp), "=c"(__d0)                          \
					 : "0"(dst_bp), "1"(src_bp), "2"(nbytes)                           \
					 : "memory");                                                      \
	} while (0)

#undef BYTE_COPY_BWD
#define BYTE_COPY_BWD(dst_ep, src_ep, nbytes)                                             \
	do                                                                                    \
	{                                                                                     \
		int __d0;                                                                         \
		asm volatile(		 /* Set the direction flag, so copying goes backwards.  */    \
					 "std\n" /* Copy bytes.  */                                           \
					 "rep\n"                                                              \
					 "movsb\n" /* Clear the dir flag.  Convention says it should be 0. */ \
					 "cld"                                                                \
					 : "=D"(dst_ep), "=S"(src_ep), "=c"(__d0)                             \
					 : "0"(dst_ep - 1), "1"(src_ep - 1), "2"(nbytes)                      \
					 : "memory");                                                         \
		dst_ep += 1;                                                                      \
		src_ep += 1;                                                                      \
	} while (0)

#undef WORD_COPY_FWD
#define WORD_COPY_FWD(dst_bp, src_bp, nbytes_left, nbytes)                             \
	do                                                                                 \
	{                                                                                  \
		int __d0;                                                                      \
		asm volatile(		 /* Clear the direction flag, so copying goes forward.  */ \
					 "cld\n" /* Copy longwords.  */                                    \
					 "rep\n"                                                           \
					 "movsl"                                                           \
					 : "=D"(dst_bp), "=S"(src_bp), "=c"(__d0)                          \
					 : "0"(dst_bp), "1"(src_bp), "2"((nbytes) / 4)                     \
					 : "memory");                                                      \
		(nbytes_left) = (nbytes) % 4;                                                  \
	} while (0)

#undef WORD_COPY_BWD
#define WORD_COPY_BWD(dst_ep, src_ep, nbytes_left, nbytes)                                \
	do                                                                                    \
	{                                                                                     \
		int __d0;                                                                         \
		asm volatile(		 /* Set the direction flag, so copying goes backwards.  */    \
					 "std\n" /* Copy longwords.  */                                       \
					 "rep\n"                                                              \
					 "movsl\n" /* Clear the dir flag.  Convention says it should be 0. */ \
					 "cld"                                                                \
					 : "=D"(dst_ep), "=S"(src_ep), "=c"(__d0)                             \
					 : "0"(dst_ep - 4), "1"(src_ep - 4), "2"((nbytes) / 4)                \
					 : "memory");                                                         \
		dst_ep += 4;                                                                      \
		src_ep += 4;                                                                      \
		(nbytes_left) = (nbytes) % 4;                                                     \
	} while (0)

#define PAGE_COPY_FWD_MAYBE(dstp, srcp, nbytes_left, nbytes) /* nada */

#endif

#ifndef INCLUDE_KERNEL_INFO_H
#define INCLUDE_KERNEL_INFO_H

#include <stdint.h>

extern void *kernel_boot;
extern void *kernel_higher_half;
extern void *kernel_start;
extern void *kernel_text_start;
extern void *kernel_text_end;
extern void *kernel_data_start;
extern void *kernel_data_end;
extern void *kernel_end;

#define KERNEL_BOOT (uint32_t)(&kernel_boot)
#define KERNEL_HIGHER_HALF (uint32_t)(&kernel_higher_half)
#define KERNEL_START (uint32_t)(&kernel_start)
#define KERNEL_TEXT_START (uint32_t)(&kernel_text_start)
#define KERNEL_TEXT_END (uint32_t)(&kernel_text_end)
#define KERNEL_DATA_START (uint32_t)(&kernel_data_start)
#define KERNEL_DATA_END (uint32_t)(&kernel_data_end)
#define KERNEL_END (uint32_t)(&kernel_end)

#endif
#ifndef MEMORY_PMM_H
#define MEMORY_PMM_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/utils/math.h>
#include <kernel/multiboot2.h>
#include <kernel/utils/string.h>
#include "kernel_info.h"

#define PMM_FRAMES_PER_BYTE 8
#define PMM_FRAME_SIZE 4096
#define PMM_FRAME_ALIGN PMM_FRAME_SIZE
#define PAGE_MASK (~(PMM_FRAME_SIZE - 1))
#define PAGE_ALIGN(addr) (((addr) + PMM_FRAME_SIZE - 1) & PAGE_MASK)

void pmm_init(struct multiboot_tag_basic_meminfo *, struct multiboot_tag_mmap *);
void *pmm_alloc_block();
void *pmm_alloc_blocks(size_t);
void pmm_free_block(void *);
void pmm_mark_used_addr(uint32_t paddr);
uint32_t get_total_frames();

#endif

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

typedef uint32_t physical_addr;

void pmm_init(struct multiboot_tag_basic_meminfo *, struct multiboot_tag_mmap *);
void *pmm_alloc_block();
void *pmm_alloc_blocks(size_t);
void pmm_free_block(void *);
void pmm_mark_used_addr(physical_addr paddr);

#endif

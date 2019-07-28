#ifndef MEMORY_PMM_H
#define MEMORY_PMM_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/include/common.h>
#include <kernel/include/kernel_info.h>
#include <kernel/include/multiboot.h>
#include <kernel/include/string.h>

#define PMM_FRAMES_PER_BYTE 8
#define PMM_FRAME_SIZE 4096
#define PMM_FRAME_ALIGN PMM_FRAME_SIZE

typedef uint32_t physical_addr;

void pmm_init(multiboot_info_t *);
void *pmm_alloc_block();
void *pmm_alloc_blocks(size_t);
void pmm_free_block(void *);

#endif

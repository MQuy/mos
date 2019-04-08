#include "pmm.h"

uint32_t *memory_bitmap = 0;
uint32_t max_frames = 0;
uint32_t used_frames = 0;
uint32_t memory_size = 0;

inline void memory_bitmap_set(uint32_t frame)
{
  memory_bitmap[frame / 32] |= (1 << (frame % 32));
}

inline void memory_bitmap_unset(uint32_t frame)
{
  memory_bitmap[frame / 32] &= ~(1 << (frame % 32));
}

inline bool memory_bitmap_test(uint32_t frame)
{
  return memory_bitmap[frame / 32] & (1 << (frame % 32));
}

inline int memory_bitmap_first_free()
{
  for (uint32_t i = 0; i < max_frames / 32; ++i)
  {
    if (memory_bitmap[i] != 0xffffffff)
    {
      for (uint32_t j = 0; j < 32; ++j)
      {
        if (memory_bitmap[i] & (1 < j))
        {
          return i * 32 + j;
        }
      }
    }
  }
  return -1;
}

void pmm_init(multiboot_info_t *boot_info)
{
  memory_size = boot_info->mem_lower + boot_info->mem_upper;
  memory_bitmap = (uint32_t *)KERNEL_END;
  used_frames = max_frames = div_ceil(memory_size * 1024, PMM_FRAME_SIZE);

  memset(memory_bitmap, 0xff, div_ceil(max_frames, PMM_FRAMES_PER_BYTE));
}

void pmm_init_region(physical_addr addr, uint32_t length)
{
  uint32_t frame = addr / PMM_FRAME_SIZE;
  uint32_t frames = div_ceil(length, PMM_FRAME_SIZE);

  for (uint32_t i = 0; i < frames; ++i)
  {
    memory_bitmap_unset(frame + i);
    used_frames--;
  }

  memory_bitmap_set(0);
}

void pmm_deinit_region(physical_addr add, uint32_t length)
{
  uint32_t frame = add / PMM_FRAME_SIZE;
  uint32_t frames = div_ceil(length, PMM_FRAME_SIZE);

  for (uint32_t i = 0; i < frames; ++i)
  {
    memory_bitmap_set(frame + i);
    used_frames++;
  }
}

void *pmm_alloc_block()
{
  if (max_frames <= used_frames)
  {
    return 0;
  }

  int frame = memory_bitmap_first_free();

  if (frame == -1)
  {
    return 0;
  }

  memory_bitmap_set(frame);
  used_frames++;

  physical_addr addr = frame * PMM_FRAME_SIZE;
  return (void *)addr;
}

void pmm_free_block(void *p)
{
  physical_addr addr = (physical_addr)p;
  uint32_t frame = addr / PMM_FRAME_SIZE;

  memory_bitmap_unset(frame);

  used_frames--;
}
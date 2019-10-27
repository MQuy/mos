#include "pmm.h"

uint32_t *memory_bitmap = 0;
uint32_t max_frames = 0;
uint32_t used_frames = 0;
uint32_t memory_size = 0;

void pmm_regions(multiboot_info_t *);
void pmm_init_region(uint32_t addr, uint32_t length);
void pmm_deinit_region(uint32_t add, uint32_t length);

void memory_bitmap_set(uint32_t frame)
{
  memory_bitmap[frame / 32] |= (1 << (frame % 32));
}

void memory_bitmap_unset(uint32_t frame)
{
  memory_bitmap[frame / 32] &= ~(1 << (frame % 32));
}

bool memory_bitmap_test(uint32_t frame)
{
  return memory_bitmap[frame / 32] & (1 << (frame % 32));
}

int memory_bitmap_first_free()
{
  for (uint32_t i = 0; i < max_frames / 32; ++i)
    if (memory_bitmap[i] != 0xffffffff)
      for (uint32_t j = 0; j < 32; ++j)
      {
        int bit = 1 << j;
        if (!(memory_bitmap[i] & bit))
          return i * 32 + j;
      }

  return -1;
}

int memory_bitmap_first_frees(size_t size)
{
  if (size == 0)
    return -1;

  if (size == 1)
    return memory_bitmap_first_free();

  for (uint32_t i = 0; i < max_frames / 32; i++)
    if (memory_bitmap[i] != 0xffffffff)
      for (int j = 0; j < 32; j++)
      { //! test each bit in the dword

        int bit = 1 << j;
        if (!(memory_bitmap[i] & bit))
        {

          int startingBit = i * 32;
          startingBit += j; //get the free bit in the dword at index i

          uint32_t free = 0; //loop through each bit to see if its enough space
          for (uint32_t count = 0; count <= size; count++)
          {

            if (!memory_bitmap_test(startingBit + count))
              free++; // this bit is clear (free frame)

            if (free == size)
              return i * 32 + j; //free count==size needed; return index
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

  pmm_regions(boot_info);
}

void pmm_regions(multiboot_info_t *boot_info)
{
  multiboot_memory_map_t *region = (multiboot_memory_map_t *)boot_info->mmap_addr;

  for (int i = 0; region < boot_info->mmap_addr + boot_info->mmap_length; i++)
  {
    if (region->type > 4 && region->addr == 0)
      break;

    if (region->type == 1)
      pmm_init_region(region->addr, region->len);

    region = (multiboot_memory_map_t *)((unsigned long)region + region->size + sizeof(region->size));
  }
  pmm_deinit_region(0x100000, KERNEL_END - KERNEL_START);
  pmm_deinit_region(0x0, 0x10000);
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
    return 0;

  int frame = memory_bitmap_first_free();

  if (frame == -1)
    return 0;

  memory_bitmap_set(frame);
  used_frames++;

  physical_addr addr = frame * PMM_FRAME_SIZE;
  return (void *)addr;
}

void *pmm_alloc_blocks(size_t size)
{
  if (max_frames - used_frames < size)
    return 0;

  int frame = memory_bitmap_first_frees(size);

  if (frame == -1)
    return 0;

  for (int i = 0; i < size; ++i)
  {
    memory_bitmap_set(frame + i);
    used_frames++;
  }

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
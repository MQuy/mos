#include "pmm.h"

static uint32_t *memory_bitmap = 0;
static uint32_t max_frames = 0;
static uint32_t used_frames = 0;
static uint32_t memory_size = 0;
static uint32_t memory_bitmap_size = 0;

void pmm_regions(struct multiboot_tag_mmap *multiboot_mmap);
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

void pmm_init(struct multiboot_tag_basic_meminfo *multiboot_meminfo, struct multiboot_tag_mmap *multiboot_mmap)
{
  memory_size = (multiboot_meminfo->mem_lower + multiboot_meminfo->mem_upper) * 1024;
  memory_bitmap = (uint32_t *)KERNEL_END;
  used_frames = max_frames = div_ceil(memory_size, PMM_FRAME_SIZE);

  memory_bitmap_size = div_ceil(max_frames, PMM_FRAMES_PER_BYTE);
  memset(memory_bitmap, 0xff, memory_bitmap_size);

  pmm_regions(multiboot_mmap);

  pmm_deinit_region(0x0, KERNEL_BOOT);
  pmm_deinit_region(KERNEL_BOOT, KERNEL_END - KERNEL_START + memory_bitmap_size);
}

void pmm_regions(struct multiboot_tag_mmap *multiboot_mmap)
{
  for (struct multiboot_mmap_entry *mmap = multiboot_mmap->entries;
       (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)multiboot_mmap + multiboot_mmap->size;
       mmap = (struct multiboot_mmap_entry *)((unsigned long)mmap + multiboot_mmap->entry_size))
  {
    if (mmap->type > 4 && mmap->addr == 0)
      break;

    if (mmap->type == 1)
      pmm_init_region(mmap->addr, mmap->len);
  }
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

void pmm_deinit_region(physical_addr addr, uint32_t length)
{
  uint32_t frame = addr / PMM_FRAME_SIZE;
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

  for (uint32_t i = 0; i < size; ++i)
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

void pmm_mark_used_addr(physical_addr paddr)
{
  uint32_t frame = paddr / PMM_FRAME_SIZE;
  if (!memory_bitmap_test(frame))
  {
    memory_bitmap_set(frame);
    used_frames++;
  }
}

uint32_t get_total_frames()
{
  return max_frames;
}
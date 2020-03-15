#include <include/errno.h>
#include <kernel/utils/string.h>
#include <stdbool.h>
#include "vmm.h"

#define BLOCK_MAGIC 0x464E

typedef struct block_meta
{
  size_t size;
  struct block_meta *next;
  bool free;
  uint32_t magic;
} block_meta;

block_meta *blocklist, *lastblock;

void validate_kblock(block_meta *block)
{
  if (block->magic != BLOCK_MAGIC)
    *(uint32_t *)BLOCK_MAGIC;
}

block_meta *find_free_block(block_meta **last, size_t size)
{
  block_meta *current = (block_meta *)blocklist;
  while (current && !(current->free && current->size >= size))
  {
    *last = current;
    current = current->next;
  }
  return current;
}

void split_block(block_meta *block, size_t size)
{
  if (block->size > size + sizeof(block_meta))
  {
    struct block_meta *splited_block = (struct block_meta *)((char *)block + size + sizeof(struct block_meta));
    splited_block->free = true;
    splited_block->magic = BLOCK_MAGIC;
    splited_block->size = block->size - size - sizeof(struct block_meta);
    splited_block->next = block->next;

    block->size = size;
    block->next = splited_block;
  }
}

block_meta *request_space(block_meta *last, size_t size)
{
  block_meta *block = sbrk(size + sizeof(block_meta));

  if (last)
    last->next = block;

  block->size = size;
  block->next = NULL;
  block->free = false;
  block->magic = BLOCK_MAGIC;
  return block;
}

void *kmalloc(size_t size)
{
  if (size <= 0)
    return NULL;

  block_meta *block, *last;

  if (blocklist)
  {
    block = find_free_block(&last, size);
    if (block)
    {
      block->free = false;
      split_block(block, size);
    }
    else
      block = request_space(lastblock, size);
  }
  else
  {
    block = request_space(NULL, size);
    blocklist = block;
  }

  lastblock = block;
  validate_kblock(block);

  if (block)
    return block + 1;
  else
    return NULL;
}

void *kcalloc(size_t n, size_t size)
{
  void *block = kmalloc(n * size);
  if (!block)
    memset(block, 0, n * size);
  return block;
}

struct block_meta *get_block_ptr(void *ptr)
{
  return (struct block_meta *)ptr - 1;
}

void kfree(void *ptr)
{
  if (!ptr)
    return;

  block_meta *block = get_block_ptr(ptr);
  validate_kblock(block);
  block->free = true;
}

// NOTE: MQ 2019-11-24
// next object in heap space can be any number
// align heap so the next object's addr will be started at size * n
// ------------------- 0xE0000000
// |                 |
// |                 |
// |                 | new object address m = (size * n)
// ------------------- m - sizeof(block_meta)
// |                 | empty object (>= 1)
// ------------------- padding - sizeof(block_meta)
void *kalign_heap(size_t size)
{
  uint32_t heap_addr = sbrk(0);

  if (heap_addr % size == 0)
    return NULL;

  uint32_t padding_size = div_ceil(heap_addr, size) * size - heap_addr;
  uint32_t required_size = sizeof(block_meta) * 2;

  while (padding_size <= KERNEL_HEAP_TOP)
  {
    if (padding_size > required_size)
      return kmalloc(padding_size - required_size);
    padding_size += size;
  }
  return NULL;
}

void *krealloc(void *ptr, size_t size)
{
  if (!ptr && size == 0)
  {
    kfree(ptr);
    return NULL;
  }
  else if (!ptr)
    return kmalloc(size);

  void *newptr = kmalloc(size);
  memcpy(newptr, ptr, size);
  return newptr;
}
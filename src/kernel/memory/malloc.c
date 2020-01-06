#include <include/errno.h>
#include <kernel/utils/string.h>
#include "vmm.h"
#include "malloc.h"

block_meta *blocklist;

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

block_meta *request_space(block_meta *last, size_t size)
{
  block_meta *block = sbrk(size + sizeof(block_meta));

  if (last)
    last->next = block;

  block->size = size;
  block->next = NULL;
  block->free = false;
  return block;
}

void *malloc(size_t size)
{
  if (size <= 0)
    return NULL;

  block_meta *block, *last;

  if (blocklist)
  {
    block = find_free_block(&last, size);
    if (block)
      block->free = false;
    else
    {
      block = request_space(last, size);
    }
  }
  else
  {
    block = request_space(NULL, size);
    blocklist = block;
  }

  if (block)
    return block + 1;
  else
    return NULL;
}

void *calloc(size_t n, size_t size)
{
  void *block = malloc(n * size);
  if (!block)
    memset(block, 0, n * size);
  return block;
}

void free(void *ptr)
{
  if (!ptr)
    return;

  block_meta *block = (block_meta *)ptr;
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
void *align_heap(size_t size)
{
  uint32_t heap_addr = sbrk(0);

  if (heap_addr % size == 0)
    return NULL;

  uint32_t padding_size = div_ceil(heap_addr, size) * size - heap_addr;
  uint32_t required_size = sizeof(block_meta) * 2;

  while (padding_size <= KERNEL_HEAP_TOP)
  {
    if (padding_size > required_size)
      return malloc(padding_size - required_size);
    padding_size += size;
  }
  return NULL;
}

void *realloc(void *ptr, size_t size)
{
  if (!ptr && size == 0)
  {
    free(ptr);
    return NULL;
  }
  else if (!ptr)
    return malloc(size);

  void *newptr = malloc(size);
  memcpy(newptr, ptr, size);
  return newptr;
}
#include <kernel/include/errno.h>
#include <kernel/include/string.h>
#include "vmm.h"
#include "malloc.h"

block_meta *blocklist, *last_block;

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
  if (size < 0)
    return -EINVAL;
  if (size == 0)
    return (char *)last_block + sizeof(block_meta) + last_block->size;

  block_meta *block, *last;

  if (blocklist)
  {
    block = find_free_block(&last, size);
    if (block)
      block->free = false;
    else
    {
      block = request_space(last, size);
      last_block = block;
    }
  }
  else
  {
    block = request_space(NULL, size);
    blocklist = last_block = block;
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
  uint32_t heap_addr = malloc(0);
  uint32_t padding_size = div_ceil(heap_addr, size) * size - heap_addr;
  uint32_t required_size = sizeof(block_meta) * 2;

  while (padding_size <= KERNEL_HEAP_TOP)
  {
    if (padding_size > required_size)
      return malloc(padding_size - required_size);
    padding_size += size;
  }
}
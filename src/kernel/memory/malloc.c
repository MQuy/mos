#include <kernel/include/string.h>
#include "vmm.h"
#include "malloc.h"

void *blocklist;

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
      block = request_space(last, size);
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
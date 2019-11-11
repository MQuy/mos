#ifndef MEMORY_MALLOC_H
#define MEMORY_MALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct block_meta
{
  size_t size;
  struct block_meta *next;
  bool free;
} block_meta;

void *malloc(size_t n);
void *calloc(size_t n, size_t size);
void free(void *ptr);

#endif
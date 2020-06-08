#ifndef LIBC_STDLIB_H
#define LIBC_STDLIB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);

#endif

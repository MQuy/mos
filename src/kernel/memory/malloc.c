#include <include/errno.h>
#include <stdbool.h>
#include <utils/math.h>
#include <utils/printf.h>
#include <utils/string.h>

#include "vmm.h"

#define BLOCK_MAGIC 0x464E

extern uint32_t heap_current;

struct block_meta
{
	size_t size;
	struct block_meta *next;
	bool free;
	uint32_t magic;
};

static struct block_meta *kblocklist = NULL;

void assert_kblock_valid(struct block_meta *block)
{
	// NOTE: MQ 2020-06-06 if a block's size > 32 MiB -> might be an corrupted block
	if (block->magic != BLOCK_MAGIC || block->size > 0x2000000)
		__asm__ __volatile("int $0x01");
}

struct block_meta *find_free_block(struct block_meta **last, size_t size)
{
	struct block_meta *current = (struct block_meta *)kblocklist;
	while (current && !(current->free && current->size >= size))
	{
		assert_kblock_valid(current);
		*last = current;
		current = current->next;
		if (current)
			assert_kblock_valid(current);
	}
	return current;
}

void split_block(struct block_meta *block, size_t size)
{
	if (block->size > size + sizeof(struct block_meta))
	{
		struct block_meta *splited_block = (struct block_meta *)((char *)block + sizeof(struct block_meta) + size);
		splited_block->free = true;
		splited_block->magic = BLOCK_MAGIC;
		splited_block->size = block->size - size - sizeof(struct block_meta);
		splited_block->next = block->next;

		block->size = size;
		block->next = splited_block;
	}
}

struct block_meta *request_space(struct block_meta *last, size_t size)
{
	struct block_meta *block = sbrk(size + sizeof(struct block_meta));

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

	struct block_meta *block;

	if (kblocklist)
	{
		struct block_meta *last = kblocklist;
		block = find_free_block(&last, size);
		if (block)
		{
			block->free = false;
			split_block(block, size);
		}
		else
			block = request_space(last, size);
	}
	else
	{
		block = request_space(NULL, size);
		kblocklist = block;
	}

	assert_kblock_valid(block);

	if (block)
		return block + 1;
	else
		return NULL;
}

void *kcalloc(size_t n, size_t size)
{
	void *block = kmalloc(n * size);
	if (block)
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

	struct block_meta *block = get_block_ptr(ptr);
	assert_kblock_valid(block);
	block->free = true;
}

// NOTE: MQ 2019-11-24
// next object in heap space can be any number
// align heap so the next object's addr will be started at size * n
// ------------------- 0xE0000000
// |                 |
// |                 |
// |                 | new object address m = (size * n)
// ------------------- m - sizeof(struct block_meta)
// |                 | empty object (>= 1)
// ------------------- padding - sizeof(struct block_meta)
void *kalign_heap(size_t size)
{
	uint32_t heap_addr = (uint32_t)sbrk(0);

	if (heap_addr % size == 0)
		return NULL;

	uint32_t padding_size = div_ceil(heap_addr, size) * size - heap_addr;
	uint32_t required_size = sizeof(struct block_meta) * 2;

	while (padding_size <= KERNEL_HEAP_TOP)
	{
		if (padding_size > required_size)
		{
			struct block_meta *last = kblocklist;
			while (!last->next)
				last = last->next;
			struct block_meta *block = request_space(last, padding_size - required_size);
			return block + 1;
		}
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
		return kcalloc(size, sizeof(char));

	void *newptr = kcalloc(size, sizeof(char));
	memcpy(newptr, ptr, size);
	return newptr;
}

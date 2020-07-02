#include <include/errno.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>

#define BLOCK_MAGIC 0x464E

static uint32_t remaining_from_last_used = 0;
static uint32_t heap_current = 0;

struct block_meta
{
	size_t size;
	struct block_meta *next;
	bool free;
	uint32_t magic;
};

static struct block_meta *blocklist = NULL;

void assert_block_valid(struct block_meta *block)
{
	if (block->magic != BLOCK_MAGIC)
		__asm__ __volatile("int $0x0E");
}

struct block_meta *find_free_block(struct block_meta **last, size_t size)
{
	uint32_t bcount = 0;
	struct block_meta *current = (struct block_meta *)blocklist;
	while (current && !(current->free && current->size >= size))
	{
		bcount++;
		assert_block_valid(current);
		*last = current;
		current = current->next;
		if (current)
			assert_block_valid(current);
	}
	return current;
}

void split_block(struct block_meta *block, size_t size)
{
	if (block->size > size + sizeof(struct block_meta))
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

void *get_heap(uint32_t size)
{
	if (!heap_current)
		heap_current = sbrk(0);

	uint32_t heap_base = heap_current;
	if (size <= remaining_from_last_used)
		remaining_from_last_used -= size;
	else
	{
		uint32_t new_brk = sbrk(size);
		remaining_from_last_used = new_brk - (heap_current + size);
	}

	heap_current += size;
	return (void *)heap_base;
}

struct block_meta *request_space(struct block_meta *last, size_t size)
{
	struct block_meta *block = get_heap(size + sizeof(struct block_meta));

	if (last)
		last->next = block;

	block->size = size;
	block->next = NULL;
	block->free = false;
	block->magic = BLOCK_MAGIC;
	return block;
}

void *malloc(size_t size)
{
	if (size <= 0)
		return NULL;

	struct block_meta *block, *last;

	if (blocklist)
	{
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
		blocklist = block;
	}

	assert_block_valid(block);

	if (block)
		return block + 1;
	else
		return NULL;
}

void *calloc(size_t n, size_t size)
{
	void *block = malloc(n * size);
	if (block)
		memset(block, 0, n * size);
	return block;
}

struct block_meta *get_block_ptr(void *ptr)
{
	return ((struct block_meta *)ptr) - 1;
}

void free(void *ptr)
{
	if (!ptr)
		return;

	struct block_meta *block = get_block_ptr(ptr);
	assert_block_valid(block);
	block->free = true;
}

void *realloc(void *ptr, size_t size)
{
	if (!ptr && size == 0)
	{
		free(ptr);
		return NULL;
	}
	else if (!ptr)
		return calloc(size, sizeof(char));

	void *newptr = calloc(size, sizeof(char));
	memcpy(newptr, ptr, size);
	return newptr;
}

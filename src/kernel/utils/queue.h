#ifndef UTILS_QUEUE_H
#define UTILS_QUEUE_H

#include <include/list.h>
#include <stdint.h>

struct queue
{
	struct list_head *qhead;
	uint32_t number_of_items;
};

void queue_push(struct queue *q, void *data);
void *queue_pop(struct queue *q);
void *queue_peek(struct queue *q);

#endif

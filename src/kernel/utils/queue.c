#include "queue.h"

#include <include/list.h>
#include <kernel/memory/vmm.h>

struct qelement
{
	void *data;
	struct list_head sibling;
};

void queue_push(struct queue *q, void *data)
{
	struct qelement *qe = kcalloc(1, sizeof(struct qelement));
	qe->data = data;
	list_add_tail(&qe->sibling, q->qhead);
}

void *queue_pop(struct queue *q)
{
	struct qelement *qe = list_first_entry(q->qhead, struct qelement, sibling);
	list_del(&qe->sibling);
	return qe->data;
}

void *queue_peek(struct queue *q)
{
	struct qelement *qe = list_first_entry(q->qhead, struct qelement, sibling);
	return qe->data;
}

#include "queue.h"
#include <kernel/memory/vmm.h>

static struct qelement
{
  void *data;
  struct list_head *sibling;
};

void queue_push(struct queue *q, void *data)
{
  struct qelement *qe = kcalloc(1, sizeof(struct qelement));
  qe->data = data;
  list_add_tail(q->qhead, qe->sibling);
}

void *queue_pop(struct queue *q)
{
  struct qelement *qe = list_first_entry(q->qhead, struct qelement, sibling);
  list_del(qe);
  return qe->data;
}

void *queue_peek(struct queue *q)
{
  struct qelement *qe = list_first_entry(q->qhead, struct qelement, sibling);
  return qe->data;
}
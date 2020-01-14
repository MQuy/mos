#include <kernel/utils/hashmap.h>
#include <kernel/locking/semaphore.h>
#include <kernel/memory/vmm.h>
#include <include/errno.h>
#include "message_queue.h"

extern thread *current_thread;

semaphore mq_locking;
struct hashmap mq_map;

void mq_init()
{
  sema_init(&mq_locking, 1);
  hashmap_init(&mq_map, hashmap_hash_string, hashmap_compare_string, 0);
}

int32_t mq_open(const char *name, int32_t flags)
{
  message_queue *mq = kmalloc(sizeof(message_queue));
  INIT_LIST_HEAD(&mq->senders);
  INIT_LIST_HEAD(&mq->receivers);
  INIT_LIST_HEAD(&mq->messages);

  if (!hashmap_put(&mq_map, name, mq))
    return -EINVAL;

  return 0;
}

int32_t mq_close(const char *name)
{
  int32_t ret = 0;
  message_queue *mq = hashmap_get(&mq_map, name);
  if (mq)
  {
    hashmap_remove(&mq_map, name);
    kfree(mq);
  }
  return ret;
}

int32_t mq_send(const char *name, char *user_buf, int32_t mtype, uint32_t msize)
{
  message_queue *mq = hashmap_get(&mq_map, name);

  if (!mq)
    return -EINVAL;

  char *kernel_buf = kmalloc(msize);
  memcpy(kernel_buf, user_buf, msize);

  mq_receiver *iter = NULL;
  mq_receiver *mqr = NULL;
  list_for_each_entry(iter, &mq->receivers, sibling)
  {
    if (iter->mtype == mtype)
    {
      mqr = iter;
      break;
    }
  }

  if (mqr)
  {
    mqr->sender = current_thread;
    mqr->msize = msize;
    mqr->buf = kernel_buf;
    list_del(&mqr->sibling);
    update_thread(mqr->receiver, THREAD_READY);
  }
  else
  {
    mq_message *msg = kmalloc(sizeof(mq_message));
    msg->buf = kernel_buf;
    msg->msize = msize;
    msg->mtype = mtype;
    msg->sender = current_thread;
    list_add_tail(&msg->sibling, &mq->messages);
  }

  if (mtype >= 0)
  {
    // NOTE: MQ 2020-01-06 Negative thread id number is reserved for ack channel
    // waiting to receive an ack from receiver
    return mq_receive(name, NULL, -current_thread->tid, 0);
  }
  else
    return 0;
}

int32_t mq_receive(const char *name, char *user_buf, int32_t mtype, uint32_t msize)
{
  message_queue *mq = hashmap_get(&mq_map, name);

  if (!mq)
    return -EINVAL;

  mq_message *iter = NULL;
  mq_message *mqm = NULL;
  list_for_each_entry(iter, &mq->messages, sibling)
  {
    if (iter->mtype == mtype)
    {
      mqm = iter;
      break;
    }
  }

  char *buf = NULL;
  tid_t sender_id;
  if (mqm)
  {
    buf = mqm->buf;
    sender_id = mqm->sender->tid;
  }
  else
  {
    mq_receiver *mqr = kmalloc(sizeof(mq_receiver));
    mqr->mtype = mtype;
    mqr->msize = msize;
    mqr->receiver = current_thread;
    list_add_tail(&mqr->sibling, &mq->receivers);
    update_thread(current_thread, THREAD_WAITING);
    schedule();

    if (!mqr->buf)
      return -EINVAL;

    buf = mqr->buf;
    sender_id = mqr->sender->tid;
  }

  // send ack signal to sender
  mq_send(name, NULL, -sender_id, 0);
  memcpy(user_buf, buf, msize);

  return 0;
}
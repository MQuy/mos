#ifndef IPC_MESSAGE_QUEUE_H
#define IPC_MESSAGE_QUEUE_H

#include <stddef.h>
#include <stdint.h>
#include <include/list.h>
#include <kernel/proc/task.h>

struct mq_sender
{
  struct thread *sender;
  size_t msize;
  int32_t mtype;
  char *buf;
  struct list_head sibling;
};

struct mq_receiver
{
  struct thread *receiver;
  struct thread *sender;
  char *buf;
  int32_t mtype;
  uint32_t msize;
  struct list_head sibling;
};

struct mq_message
{
  struct thread *sender;
  char *buf;
  int32_t mtype;
  uint32_t msize;
  struct list_head sibling;
};

struct message_queue
{
  struct list_head messages;
  struct list_head receivers;
  struct list_head senders;
};

void mq_init();
int32_t mq_open(const char *name, int32_t flags);
int32_t mq_close(const char *name);
int32_t mq_enqueue(const char *name, char *kernel_buf, int32_t mtype, uint32_t msize);
int32_t mq_send(const char *name, char *buf, int32_t mtype, uint32_t msize);
int32_t mq_receive(const char *name, char *buf, int32_t mtype, uint32_t msize);

#endif
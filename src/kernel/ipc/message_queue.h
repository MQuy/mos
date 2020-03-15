#ifndef IPC_MESSAGE_QUEUE_H
#define IPC_MESSAGE_QUEUE_H

#include <stddef.h>
#include <stdint.h>
#include <include/list.h>
#include <kernel/proc/task.h>

typedef struct mq_sender
{
  thread *sender;
  size_t msize;
  int32_t mtype;
  char *buf;
  list_head sibling;
} mq_sender;

typedef struct mq_receiver
{
  thread *receiver;
  thread *sender;
  char *buf;
  int32_t mtype;
  uint32_t msize;
  list_head sibling;
} mq_receiver;

typedef struct mq_message
{
  thread *sender;
  char *buf;
  int32_t mtype;
  uint32_t msize;
  list_head sibling;
} mq_message;

typedef struct message_queue
{
  list_head messages;
  list_head receivers;
  list_head senders;
} message_queue;

void mq_init();
int32_t mq_open(const char *name, int32_t flags);
int32_t mq_close(const char *name);
int32_t mq_enqueue(const char *name, char *kernel_buf, int32_t mtype, uint32_t msize);
int32_t mq_send(const char *name, char *buf, int32_t mtype, uint32_t msize);
int32_t mq_receive(const char *name, char *buf, int32_t mtype, uint32_t msize);

#endif
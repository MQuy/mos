#ifndef FS_PIPE_H
#define FS_PIPE_H

#include <kernel/utils/circular_buffer.h>
#include <kernel/locking/semaphore.h>
#include "vfs.h"

#define PIPE_SIZE 0x10000
#define O_RDONLY 0x0000
#define O_WRONLY 0x0001

typedef struct pipe
{
  cbuf_handle_t buf;
  semaphore mutex;
  uint32_t readers;
  uint32_t writers;
} pipe;

#endif
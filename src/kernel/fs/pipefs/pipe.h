#ifndef FS_PIPE_H
#define FS_PIPE_H

#include <kernel/utils/circular_buffer.h>
#include <kernel/locking/semaphore.h>
#include "kernel/fs/vfs.h"

#define PIPE_SIZE 0x10000

typedef struct pipe
{
  cbuf_handle_t buf;
  semaphore mutex;
  uint32_t files;
  uint32_t readers;
  uint32_t writers;
} pipe;

int do_pipe(int *fd);

#endif
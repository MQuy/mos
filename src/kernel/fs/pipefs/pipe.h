#ifndef FS_PIPE_H
#define FS_PIPE_H

#include <kernel/locking/semaphore.h>
#include <kernel/utils/circular_buffer.h>

#include "kernel/fs/vfs.h"

#define PIPE_SIZE 0x10000

struct pipe
{
	struct circular_buf_t *buf;
	struct semaphore mutex;
	uint32_t files;
	uint32_t readers;
	uint32_t writers;
};

int32_t do_pipe(int32_t *fd);

#endif

#include <stdint.h>
#include <stddef.h>
#include <kernel/memory/vmm.h>

#include "circular_buffer.h"

// The definition of our circular buffer structure is hidden from the user
struct circular_buf_t
{
  uint8_t *buffer;
  size_t head;
  size_t tail;
  size_t max; //of the buffer
  bool full;
};

static void advance_pointer(cbuf_handle_t cbuf)
{
  if (cbuf->full)
  {
    cbuf->tail = (cbuf->tail + 1) % cbuf->max;
  }

  cbuf->head = (cbuf->head + 1) % cbuf->max;

  // We mark full because we will advance tail on the next time around
  cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer(cbuf_handle_t cbuf)
{
  cbuf->full = false;
  cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}

cbuf_handle_t circular_buf_init(uint8_t *buffer, size_t size)
{
  cbuf_handle_t cbuf = kmalloc(sizeof(circular_buf_t));

  cbuf->buffer = buffer;
  cbuf->max = size;
  circular_buf_reset(cbuf);

  return cbuf;
}

void circular_buf_free(cbuf_handle_t cbuf)
{
  kfree(cbuf);
}

void circular_buf_reset(cbuf_handle_t cbuf)
{
  cbuf->head = 0;
  cbuf->tail = 0;
  cbuf->full = false;
}

size_t circular_buf_size(cbuf_handle_t cbuf)
{
  size_t size = cbuf->max;

  if (!cbuf->full)
  {
    if (cbuf->head >= cbuf->tail)
    {
      size = (cbuf->head - cbuf->tail);
    }
    else
    {
      size = (cbuf->max + cbuf->head - cbuf->tail);
    }
  }

  return size;
}

size_t circular_buf_capacity(cbuf_handle_t cbuf)
{
  return cbuf->max;
}

void circular_buf_put(cbuf_handle_t cbuf, uint8_t data)
{
  cbuf->buffer[cbuf->head] = data;

  advance_pointer(cbuf);
}

int circular_buf_put2(cbuf_handle_t cbuf, uint8_t data)
{
  int r = -1;

  if (!circular_buf_full(cbuf))
  {
    cbuf->buffer[cbuf->head] = data;
    advance_pointer(cbuf);
    r = 0;
  }

  return r;
}

int circular_buf_get(cbuf_handle_t cbuf, uint8_t *data)
{
  int r = -1;

  if (!circular_buf_empty(cbuf))
  {
    *data = cbuf->buffer[cbuf->tail];
    retreat_pointer(cbuf);

    r = 0;
  }

  return r;
}

bool circular_buf_empty(cbuf_handle_t cbuf)
{
  return (!cbuf->full && (cbuf->head == cbuf->tail));
}

bool circular_buf_full(cbuf_handle_t cbuf)
{
  return cbuf->full;
}
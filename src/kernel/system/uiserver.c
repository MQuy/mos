#include <kernel/memory/vmm.h>
#include <include/mman.h>
#include <include/msgui.h>
#include <kernel/ipc/message_queue.h>
#include <kernel/utils/queue.h>
#include <kernel/utils/string.h>
#include "uiserver.h"

extern struct thread *current_thread;
static struct thread *wsthread;

void uiserver_init(struct thread *t)
{
  wsthread = t;
}

void enqueue_event(struct msgui_event *event)
{
  struct msgui *msgui = kcalloc(1, sizeof(struct msgui));
  msgui->type = MSGUI_EVENT;
  memcpy(msgui->data, event, sizeof(struct msgui_event));
  if (current_thread == wsthread || current_thread->state == THREAD_WAITING)
    mq_enqueue(WINDOW_SERVER_SHM, (char *)msgui, 0, sizeof(struct msgui));
  else
  {
    mq_send(WINDOW_SERVER_SHM, (char *)msgui, 0, sizeof(struct msgui));
  }
}

void enqueue_mouse_event(struct mouse_device *md)
{
  struct msgui_event *event = kcalloc(1, sizeof(struct msgui_event));
  event->mouse_x = md->x;
  event->mouse_y = md->y;
  event->mouse_state = md->state;
  event->type = MSGUI_MOUSE;
  enqueue_event(event);
}

void enqueue_keyboard_event(int32_t key)
{
  struct msgui_event *event = kcalloc(1, sizeof(struct msgui_event));
  event->key = key;
  event->type = MSGUI_KEYBOARD;
  enqueue_event(event);
}

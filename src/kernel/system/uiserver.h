#ifndef SYSTEM_UISERVER_H
#define SYSTEM_UISERVER_H

#include <kernel/devices/mouse.h>
#include <kernel/proc/task.h>
#include <stdint.h>

void uiserver_init(struct thread *t);
void enqueue_mouse_event(struct mouse_device *md);
void enqueue_keyboard_event(int32_t key);

#endif

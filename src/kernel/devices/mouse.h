#ifndef DEVICES_MOUSE_H
#define DEVICES_MOUSE_H

#include <include/list.h>
#include <stdbool.h>
#include <stdint.h>

#define MOUSE_PACKET_QUEUE_LEN 16
#define MOUSE_MAJOR 13

#define BUTTON_LEFT 0x01
#define BUTTON_RIGHT 0x02
#define BUTTON_MIDDLE 0x04

#define BUTTON_LEFT_MASK (1 << 4)
#define BUTTON_RIGHT_MASK (1 << 5)
#define BUTTON_MIDDLE_MASK (1 << 6)

struct mouse_event
{
	int32_t x;
	int32_t y;
	uint8_t buttons;
	unsigned int state;
} mouse_event;

struct mouse_inode
{
	bool ready;
	struct mouse_event packets[MOUSE_PACKET_QUEUE_LEN];	 // only store 16 latest mouse motions
	uint8_t head, tail;
	struct list_head sibling;
	struct vfs_file *file;
};

void mouse_init();
void mouse_notify_readers(struct mouse_event *mm);

#endif

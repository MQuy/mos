#ifndef DEVICES_MOUSE_H
#define DEVICES_MOUSE_H

#include <include/list.h>
#include <stdbool.h>
#include <stdint.h>

#define MOUSE_PACKET_QUEUE_LEN 16
#define MOUSE_MAJOR 13
#define MOUSE_LEFT_CLICK 0x01
#define MOUSE_RIGHT_CLICK 0x02
#define MOUSE_MIDDLE_CLICK 0x04

struct mouse_event
{
	uint32_t x;
	uint32_t y;
	uint32_t buttons;
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

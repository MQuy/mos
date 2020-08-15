#ifndef DEVICES_MOUSE_H
#define DEVICES_MOUSE_H

#include <include/list.h>
#include <stdbool.h>
#include <stdint.h>

#define PACKET_QUEUE_LEN 16
#define MOUSE_MAJOR 13

struct mouse_motion
{
	uint32_t x;
	uint32_t y;
	uint32_t buttons;
} mouse_motion;

struct mouse_inode
{
	bool ready;
	struct mouse_motion packets[PACKET_QUEUE_LEN];	// only store 16 latest mouse motions
	uint8_t head, tail;
	struct list_head sibling;
	struct vfs_file *file;
};

void mouse_init();

#endif

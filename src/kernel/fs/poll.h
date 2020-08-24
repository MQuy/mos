#ifndef FS_POLL_H
#define FS_POLL_H

#include <include/list.h>
#include <kernel/proc/wait.h>
#include <stdint.h>

#define POLLIN 0x0001
#define POLLPRI 0x0002
#define POLLOUT 0x0004
#define POLLERR 0x0008
#define POLLHUP 0x0010
#define POLLNVAL 0x0020
#define POLLRDNORM 0x0040
#define POLLRDBAND 0x0080
#define POLLWRNORM 0x0100
#define POLLWRBAND 0x0200
#define POLLMSG 0x0400
#define POLLREMOVE 0x1000

struct poll_table
{
	struct list_head list;
};

struct poll_table_entry
{
	struct vfs_file *file;
	struct wait_queue_entry wait;
	struct list_head sibling;
};

struct pollfd
{
	int32_t fd;		 /* file descriptor */
	int16_t events;	 /* requested events */
	int16_t revents; /* returned events */
};

int do_poll(struct pollfd *fds, uint32_t nfds);
void poll_wait(struct vfs_file *file, struct wait_queue_head *wh, struct poll_table *pt);
void poll_wakeup(struct thread *t);

#endif

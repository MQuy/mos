#ifndef FS_POLL_H
#define FS_POLL_H

#include <include/list.h>
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

struct thread;

typedef void (*wait_queue_func)(struct thread *);

struct wait_queue_head
{
	struct list_head list;
};

struct wait_queue_entry
{
	struct thread *thread;
	wait_queue_func func;
	struct list_head sibling;
};

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

extern volatile struct thread *current_thread;
extern void schedule();

#define DEFINE_WAIT(name)            \
	struct wait_queue_entry name = { \
		.thread = current_thread,    \
		.func = poll_wakeup,         \
	}

// NOTE: MQ 2020-08-17 continue if receiving a signal
#define wait_until(cond)                               \
	for (; !(cond);)                                   \
	{                                                  \
		update_thread(current_thread, THREAD_WAITING); \
		schedule();                                    \
	}

#define wait_until_with_prework(cond, prework)         \
	for (; !(cond);)                                   \
	{                                                  \
		prework;                                       \
		update_thread(current_thread, THREAD_WAITING); \
		schedule();                                    \
	}

#define wait_event(wh, cond) ({                  \
	DEFINE_WAIT(__wait);                         \
	list_add_tail(&__wait.sibling, &(wh)->list); \
	wait_until(cond);                            \
	list_del(&__wait.sibling);                   \
})

#endif

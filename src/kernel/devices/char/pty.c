#include "tty.h"

#define NR_PTY_MAX (1 << MINORBITS)

struct tty_driver *ptm_driver, *pts_driver;
volatile int next_pty_number = 0;

int get_next_pty_number()
{
	// TODO: MQ 2020-09-04 Use bitmap to get/check next an unused number, release/uncheck a number
	return next_pty_number++;
}

int pty_open(struct tty_struct *tty, struct vfs_file *filp)
{
	return 0;
}

int pty_write(struct tty_struct *tty, const char *buf, int count)
{
	struct tty_struct *to = tty->link;

	if (!to)
		return 0;

	int c = to->ldisc->receive_room(to);
	if (c > count)
		c = count;
	to->ldisc->receive_buf(to, buf, c);
	return c;
}

int pty_write_room(struct tty_struct *tty)
{
	struct tty_struct *to = tty->link;

	if (!to)
		return 0;

	return to->ldisc->receive_room(to);
}

struct tty_operations pty_ops = {
	.open = pty_open,
	.write = pty_write,
	.write_room = pty_write_room,
};

void pty_init()
{
	ptm_driver = alloc_tty_driver(NR_PTY_MAX);
	ptm_driver->driver_name = "pty_master";
	ptm_driver->name = "ptm";
	ptm_driver->major = UNIX98_PTY_MASTER_MAJOR;
	ptm_driver->minor_start = 0;
	ptm_driver->type = TTY_DRIVER_TYPE_PTY;
	ptm_driver->subtype = PTY_TYPE_MASTER;
	ptm_driver->init_termios = tty_std_termios;
	ptm_driver->init_termios.c_iflag = 0;
	ptm_driver->init_termios.c_oflag = 0;
	ptm_driver->init_termios.c_cflag = B38400 | CS8 | CREAD;
	ptm_driver->init_termios.c_lflag = 0;
	ptm_driver->flags = TTY_DRIVER_RESET_TERMIOS | TTY_DRIVER_REAL_RAW |
						TTY_DRIVER_NO_DEVFS | TTY_DRIVER_DEVPTS_MEM;
	ptm_driver->tops = &pty_ops;
	ptm_driver->other = pts_driver;
	INIT_LIST_HEAD(&ptm_driver->ttys);
	tty_register_driver(ptm_driver);

	pts_driver = alloc_tty_driver(NR_PTY_MAX);
	pts_driver->driver_name = "pty_slave";
	pts_driver->name = "pts";
	pts_driver->major = UNIX98_PTY_SLAVE_MAJOR;
	pts_driver->minor_start = 0;
	pts_driver->type = TTY_DRIVER_TYPE_PTY;
	pts_driver->subtype = PTY_TYPE_SLAVE;
	pts_driver->init_termios = tty_std_termios;
	pts_driver->init_termios.c_cflag = B38400 | CS8 | CREAD;
	pts_driver->flags = TTY_DRIVER_RESET_TERMIOS | TTY_DRIVER_REAL_RAW |
						TTY_DRIVER_NO_DEVFS | TTY_DRIVER_DEVPTS_MEM;
	pts_driver->tops = &pty_ops;
	pts_driver->other = ptm_driver;
	INIT_LIST_HEAD(&pts_driver->ttys);
	tty_register_driver(pts_driver);
}

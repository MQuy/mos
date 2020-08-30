#include "tty.h"

#define NR_PTY_MAX (1 << MINORBITS)

struct tty_driver *ptm_driver, *pts_driver;
volatile uint32_t next_pty_number = 0;

uint32_t get_next_pty_number()
{
	return next_pty_number++;
}

int pty_open(struct tty_struct *tty, struct vfs_file *filp)
{
	return 0;
}

int pty_write(struct tty_struct *tty, const unsigned char *buf, int count)
{
	return 0;
}

struct tty_operations pty_ops = {
	.open = pty_open,
	.write = pty_write,
};

void pty_init()
{
	ptm_driver = alloc_tty_driver(NR_PTY_MAX);
	ptm_driver->name = "ptm";
	ptm_driver->major = PTY_MASTER_MAJOR;
	ptm_driver->minor_start = 0;
	ptm_driver->type = UNIX98_PTY_MASTER_MAJOR;
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
	tty_register_driver(ptm_driver);

	pts_driver = alloc_tty_driver(NR_PTY_MAX);
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
	tty_register_driver(pts_driver);
}

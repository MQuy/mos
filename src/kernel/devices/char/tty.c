#include "tty.h"

#include <include/errno.h>
#include <kernel/fs/vfs.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include <kernel/utils/printf.h>
#include <kernel/utils/string.h>

struct list_head tty_drivers;
struct termios tty_std_termios = {
	.c_iflag = ICRNL | IXON,
	.c_oflag = OPOST | ONLCR,
	.c_cflag = B38400 | CS8 | CREAD | HUPCL,
	.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK |
			   ECHOCTL | ECHOKE | IEXTEN,
	.c_cc = INIT_C_CC};

struct tty_struct *find_tty_from_driver(struct tty_driver *driver, int idx)
{
	struct tty_struct *iter;
	list_for_each_entry(iter, &driver->ttys, sibling)
	{
		if (iter->index == idx)
			return iter;
	}

	return NULL;
}

int init_dev(struct tty_driver *driver, int idx, struct tty_struct *tty)
{
	tty->index = idx;
	tty->pgrp = current_process->gid;
	tty->session = current_process->sid;
	tty->driver = driver;
	tty->ldisc = &tty_ldisc_N_TTY;
	tty->termios = &driver->init_termios;
	sprintf(tty->name, "%s/%d", driver->name, idx);
	list_add_tail(&tty->sibling, &driver->ttys);

	if (tty->ldisc->open)
		tty->ldisc->open(tty);

	return 0;
}

struct tty_struct *alloc_tty_struct()
{
	struct tty_struct *tty = kcalloc(1, sizeof(struct tty_struct));
	tty->magic = TTY_MAGIC;
	INIT_LIST_HEAD(&tty->read_wait.list);
	INIT_LIST_HEAD(&tty->write_wait.list);

	return tty;
}

int ptmx_open(struct vfs_inode *inode, struct vfs_file *file)
{
	int index = get_next_pty_number();
	struct tty_struct *ttym = alloc_tty_struct();
	init_dev(ptm_driver, index, ttym);
	file->private_data = ttym;
	ttym->driver->tops->open(ttym, file);

	struct tty_struct *ttys = alloc_tty_struct();
	init_dev(pts_driver, index, ttys);

	ttym->link = ttys;
	ttys->link = ttym;

	char path[64] = {0};
	sprintf(path, "/dev/%s", ttys->name);
	vfs_mknod(path, S_IFCHR, MKDEV(UNIX98_PTY_SLAVE_MAJOR, index));

	return 0;
}

int tty_open(struct vfs_inode *inode, struct vfs_file *file)
{
	struct tty_struct *tty = NULL;
	dev_t dev = inode->i_rdev;

	if (MAJOR(dev) == UNIX98_PTY_SLAVE_MAJOR)
	{
		tty = find_tty_from_driver(pts_driver, MINOR(dev));
		tty->driver->tops->open(tty, file);
	}

	file->private_data = tty;

	return 0;
}

ssize_t tty_read(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
	struct tty_struct *tty = (struct tty_struct *)file->private_data;
	struct tty_ldisc *ld = tty->ldisc;

	if (!tty || !ld->read)
		return -EIO;

	return ld->read(tty, file, buf, count);
}

ssize_t tty_write(struct vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
	struct tty_struct *tty = (struct tty_struct *)file->private_data;
	struct tty_ldisc *ld = tty->ldisc;

	if (!tty || !tty->driver->tops->write || !ld->write)
		return -EIO;

	return ld->write(tty, file, buf, count);
}

unsigned int tty_poll(struct vfs_file *file, struct poll_table *pt)
{
	struct tty_struct *tty = (struct tty_struct *)file->private_data;
	struct tty_ldisc *ld = tty->ldisc;

	if (!tty || !ld->poll)
		return -EIO;

	return ld->poll(tty, file, pt);
}

struct vfs_file_operations ptmx_fops = {
	.open = ptmx_open,
	.read = tty_read,
	.write = tty_write,
	.poll = tty_poll,
};

struct vfs_file_operations tty_fops = {
	.open = tty_open,
	.read = tty_read,
	.write = tty_write,
	.poll = tty_poll,
};

struct tty_driver *alloc_tty_driver(int32_t lines)
{
	struct tty_driver *driver = kcalloc(1, sizeof(struct tty_driver));
	driver->magic = TTY_DRIVER_MAGIC;
	driver->num = lines;

	return driver;
}

void tty_default_put_char(struct tty_struct *tty, const char ch)
{
	tty->driver->tops->write(tty, &ch, 1);
}

int tty_register_driver(struct tty_driver *driver)
{
	struct char_device *cdev = alloc_chrdev(driver->name, driver->major, driver->minor_start, driver->minor_num, &tty_fops);
	register_chrdev(cdev);
	driver->cdev = cdev;

	if (!(driver->flags & TTY_DRIVER_NO_DEVFS))
	{
		char name[64] = {0};
		for (int i = 0; i <= driver->minor_num; ++i)
		{
			memset(&name, 0, sizeof(name));
			sprintf(name, "/dev/%s%d", driver->name, driver->minor_start + i);
			vfs_mknod(name, S_IFCHR, MKDEV(driver->major, driver->minor_start + i));
		}
	}
	if (!driver->tops->put_char)
		driver->tops->put_char = tty_default_put_char;
	list_add_tail(&driver->sibling, &tty_drivers);

	return 0;
}

void tty_init()
{
	INIT_LIST_HEAD(&tty_drivers);

	struct char_device *ptmx_cdev = alloc_chrdev("ptmx", TTYAUX_MAJOR, 2, 1, &ptmx_fops);
	register_chrdev(ptmx_cdev);
	vfs_mknod("/dev/ptmx", S_IFCHR, ptmx_cdev->dev);

	pty_init();
}

#include <include/ctype.h>
#include <include/errno.h>
#include <include/types.h>
#include <ipc/signal.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <utils/debug.h>
#include <utils/math.h>

#include "tty.h"

static void put_tty_queue(struct tty_struct *tty, char ch)
{
	if (tty->read_count >= N_TTY_BUF_SIZE)
		assert_not_reached();

	if (tty->read_count)
		tty->read_tail = N_TTY_BUF_ALIGN(tty->read_tail + 1);
	else
		assert(tty->read_head == tty->read_tail);
	tty->read_buf[tty->read_tail] = ch;
	tty->read_count++;
}

static ssize_t opost_block(struct tty_struct *tty, const char *buf, ssize_t nr)
{
	int room = tty->ldisc->receive_room(tty);

	if (!room)
		return 0;
	if (nr > room)
		nr = room;

	if (O_OPOST(tty))
	{
		char *wbuf = kcalloc(1, N_TTY_BUF_SIZE);
		char *ibuf = wbuf;
		int wlength = 0;

		for (int i = 0; i < nr; ++i)
		{
			char ch = buf[i];
			if (O_ONLCR(tty) && ch == '\n')
			{
				*ibuf++ = '\r';
				*ibuf++ = ch;
				wlength += 2;
			}
			else if (O_OCRNL(tty) && ch == '\r')
			{
				*ibuf++ = '\n';
				wlength++;
			}
			else
			{
				*ibuf++ = O_OLCUC(tty) ? toupper(ch) : ch;
				wlength++;
			}
		}

		tty->driver->tops->write(tty, wbuf, wlength);
		kfree(wbuf);
		return wlength;
	}
	else
	{
		tty->driver->tops->write(tty, buf, nr);
		return nr;
	}
}

static void eraser(struct tty_struct *tty, char ch)
{
	int line_length = 0;
	for (; line_length < tty->read_count;)
	{
		if (LINE_SEPARATOR(tty, tty->read_buf[N_TTY_BUF_ALIGN(tty->read_tail - line_length)]))
			break;
		line_length++;
		if (N_TTY_BUF_ALIGN(tty->read_tail - line_length) == tty->read_head)
			break;
	}

	if (!line_length)
		return;

	int erase_length = 0;
	if (WERASE_CHAR(tty) == ch)
	{
		for (int i = 0; i < line_length; ++i)
		{
			char pch = tty->read_buf[N_TTY_BUF_ALIGN(tty->read_tail - i)];
			if (isspace(pch))
				break;
			erase_length++;
		}
	}
	else if (KILL_CHAR(tty) == ch)
		erase_length = line_length;
	else
		erase_length = 1;

	if (L_ECHO(tty) &&
		((KILL_CHAR(tty) == ch && L_ECHOK(tty)) ||
		 ((ERASE_CHAR(tty) == ch || WERASE_CHAR(tty) == ch) && L_ECHOE(tty))))
		opost_block(tty, &(const char){ch}, 1);

	tty->read_count = tty->read_count - erase_length;
	if (tty->read_head != tty->read_tail)
		tty->read_tail = N_TTY_BUF_ALIGN(tty->read_tail - erase_length);
}

static void copy_from_read_buf(struct tty_struct *tty, int length, char *buf)
{
	for (int i = 0; i < length; ++i)
		buf[i] = tty->read_buf[N_TTY_BUF_ALIGN(tty->read_head + i)];
}

static void assert_from_read_buf(struct tty_struct *tty, int length)
{
	if (!length)
		return;

	assert(length <= tty->read_count);
	if (tty->read_head != tty->read_tail)
	{
		if (tty->read_head > tty->read_tail)
			assert(tty->read_head + length - 1 <= tty->read_tail + N_TTY_BUF_SIZE);
		else
			assert(tty->read_head + length - 1 <= tty->read_tail);
	}
	else
		assert(length == 1);
}

int ntty_open(struct tty_struct *tty)
{
	tty->read_buf = kcalloc(1, N_TTY_BUF_SIZE);
	INIT_LIST_HEAD(&tty->read_wait.list);
	INIT_LIST_HEAD(&tty->write_wait.list);

	return 0;
}

void ntty_close(struct tty_struct *tty)
{
	kfree(tty->read_buf);
}

ssize_t ntty_read(struct tty_struct *tty, struct vfs_file *file, char *buf, size_t nr)
{
	if (current_process->tty == tty && current_process->gid != tty->pgrp && current_process->pid != tty->session)
	{
		do_kill(-current_process->gid, SIGTTIN);
		return -ERESTARTSYS;
	}

	DEFINE_WAIT(wait);
	list_add_tail(&wait.sibling, &tty->read_wait.list);
	int length;

	while (true)
	{
		length = 0;
		if (L_ICANON(tty) && tty->read_count)
		{
			int count = 0;
			int pos = tty->read_head;
			while (true)
			{
				count++;
				if (LINE_SEPARATOR(tty, tty->read_buf[pos]))
				{
					length = count;
					break;
				}
				if (pos == tty->read_tail)
					break;
				pos = N_TTY_BUF_ALIGN(pos + 1);
			}

			if (length)
				break;
		}
		else if (!L_ICANON(tty) && tty->read_count >= MIN_CHAR(tty))
		{
			length = tty->read_count;
			break;
		}
		update_thread(current_thread, THREAD_WAITING);
		schedule();
	}
	list_del(&wait.sibling);

	if (!length || length > tty->read_count)
		return -EFAULT;

    int count = min_t(int, length, nr);
	assert_from_read_buf(tty, count);
	copy_from_read_buf(tty, count, buf);
	if (count == tty->read_count)
	{
		tty->read_head = tty->read_tail = 0;
		tty->read_count = 0;
	}
	else
	{
		tty->read_head = N_TTY_BUF_ALIGN(tty->read_head + count);
		tty->read_count -= count;
	}
	wake_up(&tty->write_wait);

	return count;
}

ssize_t ntty_write(struct tty_struct *tty, struct vfs_file *file, const char *buf, size_t nr)
{
	if (current_process->tty == tty && current_process->gid != tty->pgrp && current_process->pid != tty->session)
	{
		do_kill(-current_process->gid, SIGTTOU);
		return -ERESTARTSYS;
	}

	DEFINE_WAIT(wait);
	list_add_tail(&wait.sibling, &tty->write_wait.list);

	while (true)
	{
		if (tty->ldisc->receive_room(tty) >= nr)
		{
			opost_block(tty, buf, nr);
			break;
		}
		update_thread(current_thread, THREAD_WAITING);
		schedule();
	}

	list_del(&wait.sibling);
	return nr;
}

int ntty_receive_room(struct tty_struct *tty)
{
	return N_TTY_BUF_SIZE - tty->read_count;
}

void ntty_receive_buf(struct tty_struct *tty, const char *cp, int count)
{
	if (L_ICANON(tty))
	{
		for (int i = 0; i < count; ++i)
		{
			char ch = cp[i];

			if (I_ISTRIP(tty))
				ch &= 0x7f;
			if (I_IUCLC(tty) && L_IEXTEN(tty))
				ch = tolower(ch);
			if (ch == '\r')
			{
				if (I_IGNCR(tty))
					continue;
				if (I_ICRNL(tty))
					ch = '\n';
			}
			else if (ch == '\n' && I_INLCR(tty))
				ch = '\r';

			if (L_ISIG(tty))
			{
				int32_t sig = -1;
				if (INTR_CHAR(tty) == ch)
					sig = SIGINT;
				else if (QUIT_CHAR(tty) == ch)
					sig = SIGQUIT;
				else if (SUSP_CHAR(tty) == ch)
					sig = SIGTSTP;

				if (valid_signal(sig) && sig > 0)
				{
					if (tty->pgrp > 0)
						do_kill(-tty->pgrp, sig);
					continue;
				}
			}

			if (L_ICANON(tty))
			{
				if (ch == ERASE_CHAR(tty) || ch == KILL_CHAR(tty) ||
					(ch == WERASE_CHAR(tty) && L_IEXTEN(tty)))
				{
					eraser(tty, ch);
					continue;
				}
				if (EOF_CHAR(tty) == ch)
				{
					put_tty_queue(tty, __DISABLED_CHAR);
					wake_up(&tty->read_wait);
					continue;
				}
				if (EOL_CHAR(tty) == ch || (ch == EOL2_CHAR(tty) && L_IEXTEN(tty)) || ch == '\n')
				{
					put_tty_queue(tty, ch);
					if (L_ECHONL(tty) && ch == '\n')
						opost_block(tty, &(const char){ch}, 1);
					wake_up(&tty->read_wait);
					continue;
				}
			}

			put_tty_queue(tty, ch);
			if (L_ECHO(tty))
				opost_block(tty, &(const char){ch}, 1);
		}
	}
	else
	{
		for (int i = 0; i < count; ++i)
			put_tty_queue(tty, cp[i]);
		if (L_ECHO(tty))
			opost_block(tty, cp, count);
		if (tty->read_count >= MIN_CHAR(tty))
			wake_up(&tty->read_wait);
	}
}

unsigned int ntty_poll(struct tty_struct *tty, struct vfs_file *file, struct poll_table *ptable)
{
	uint32_t mask = 0;

	poll_wait(file, &tty->read_wait, ptable);
	poll_wait(file, &tty->write_wait, ptable);

	if (tty->read_count)
		mask |= POLLIN | POLLRDNORM;
	if (tty->read_count < N_TTY_BUF_SIZE)
		mask |= POLLOUT | POLLWRNORM;

	return mask;
}

struct tty_ldisc tty_ldisc_N_TTY = {
	.magic = TTY_LDISC_MAGIC,
	.open = ntty_open,
	.close = ntty_close,
	.read = ntty_read,
	.write = ntty_write,
	.receive_room = ntty_receive_room,
	.receive_buf = ntty_receive_buf,
	.poll = ntty_poll,
};

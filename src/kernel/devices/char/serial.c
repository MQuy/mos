#include <kernel/cpu/hal.h>
#include <kernel/devices/char/tty.h>
#include <kernel/memory/vmm.h>

static int serports[] = {0x3f8, 0x2f8, 0x3e8, 0x2e8};
struct tty_driver *serial_driver;

static int serial_transmit_empty(int port)
{
	return inportb(port + 5) & 0x20;
}

void serial_output(int port, char a)
{
	while (serial_transmit_empty(port) == 0)
		;

	outportb(port, a);
}

void serial_enable(int port)
{
	outportb(port + 1, 0x00);  // Disable all interrupts
	outportb(port + 3, 0x80);  // Enable DLAB (set baud rate divisor)
	outportb(port + 0, 0x03);  // Set divisor to 3 (lo byte) 38400 baud
	outportb(port + 1, 0x00);  //                  (hi byte)
	outportb(port + 3, 0x03);  // 8 bits, no parity, one stop bit
	outportb(port + 2, 0xC7);  // Enable FIFO, clear them, with 14-byte threshold
	outportb(port + 4, 0x0B);  // IRQs enabled, RTS/DSR set
}

static int serial_open(struct tty_struct *tty, struct vfs_file *filp)
{
	int port = serports[tty->index];
	serial_enable(port);
	return 0;
}

static int serial_write(struct tty_struct *tty, const char *buf, int count)
{
	int port = serports[tty->index];
	for (int i = 0; i < count; ++i)
		serial_output(port, buf[i]);
	return 0;
}

static int serial_write_room(struct tty_struct *tty)
{
	return N_TTY_BUF_SIZE;
}

static struct tty_operations serial_ops = {
	.open = serial_open,
	.write = serial_write,
	.write_room = serial_write_room,
};

void serial_init()
{
	serial_driver = alloc_tty_driver(sizeof(serports) / sizeof(int));
	serial_driver->driver_name = "serial";
	serial_driver->name = "ttyS";
	serial_driver->major = TTY_MAJOR;
	serial_driver->minor_start = SERIAL_MINOR_BASE;
	serial_driver->type = TTY_DRIVER_TYPE_SERIAL;
	serial_driver->subtype = SERIAL_TYPE_NORMAL;
	serial_driver->init_termios = tty_std_termios;
	serial_driver->flags = TTY_DRIVER_RESET_TERMIOS | TTY_DRIVER_REAL_RAW | TTY_DRIVER_DEVPTS_MEM;
	serial_driver->tops = &serial_ops;
	INIT_LIST_HEAD(&serial_driver->ttys);
	tty_register_driver(serial_driver);
}

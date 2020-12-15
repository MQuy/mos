#include "kybrd.h"

#include <cpu/hal.h>
#include <cpu/idt.h>
#include <devices/kybrd.h>
#include <devices/mouse.h>
#include <fs/char_dev.h>
#include <include/bitops.h>
#include <include/errno.h>
#include <include/types.h>
#include <proc/task.h>
#include <utils/debug.h>
#include <utils/string.h>

static void kybrd_notify_readers(struct key_event *event);

// keyboard encoder ------------------------------------------

enum KYBRD_ENCODER_IO
{

	KYBRD_ENC_INPUT_BUF = 0x60,
	KYBRD_ENC_CMD_REG = 0x60
};

enum KYBRD_ENC_CMDS
{

	KYBRD_ENC_CMD_SET_LED = 0xED,
	KYBRD_ENC_CMD_ECHO = 0xEE,
	KYBRD_ENC_CMD_SCAN_CODE_SET = 0xF0,
	KYBRD_ENC_CMD_ID = 0xF2,
	KYBRD_ENC_CMD_AUTODELAY = 0xF3,
	KYBRD_ENC_CMD_ENABLE = 0xF4,
	KYBRD_ENC_CMD_RESETWAIT = 0xF5,
	KYBRD_ENC_CMD_RESETSCAN = 0xF6,
	KYBRD_ENC_CMD_ALL_AUTO = 0xF7,
	KYBRD_ENC_CMD_ALL_MAKEBREAK = 0xF8,
	KYBRD_ENC_CMD_ALL_MAKEONLY = 0xF9,
	KYBRD_ENC_CMD_ALL_MAKEBREAK_AUTO = 0xFA,
	KYBRD_ENC_CMD_SINGLE_AUTOREPEAT = 0xFB,
	KYBRD_ENC_CMD_SINGLE_MAKEBREAK = 0xFC,
	KYBRD_ENC_CMD_SINGLE_BREAKONLY = 0xFD,
	KYBRD_ENC_CMD_RESEND = 0xFE,
	KYBRD_ENC_CMD_RESET = 0xFF
};

// keyboard controller ---------------------------------------

enum KYBRD_CTRL_IO
{

	KYBRD_CTRL_STATS_REG = 0x64,
	KYBRD_CTRL_CMD_REG = 0x64
};

enum KYBRD_CTRL_STATS_MASK
{

	KYBRD_CTRL_STATS_MASK_OUT_BUF = 1,	   //00000001
	KYBRD_CTRL_STATS_MASK_IN_BUF = 2,	   //00000010
	KYBRD_CTRL_STATS_MASK_SYSTEM = 4,	   //00000100
	KYBRD_CTRL_STATS_MASK_CMD_DATA = 8,	   //00001000
	KYBRD_CTRL_STATS_MASK_LOCKED = 0x10,   //00010000
	KYBRD_CTRL_STATS_MASK_AUX_BUF = 0x20,  //00100000
	KYBRD_CTRL_STATS_MASK_TIMEOUT = 0x40,  //01000000
	KYBRD_CTRL_STATS_MASK_PARITY = 0x80	   //10000000
};

enum KYBRD_CTRL_CMDS
{

	KYBRD_CTRL_CMD_READ = 0x20,
	KYBRD_CTRL_CMD_WRITE = 0x60,
	KYBRD_CTRL_CMD_SELF_TEST = 0xAA,
	KYBRD_CTRL_CMD_INTERFACE_TEST = 0xAB,
	KYBRD_CTRL_CMD_DISABLE = 0xAD,
	KYBRD_CTRL_CMD_ENABLE = 0xAE,
	KYBRD_CTRL_CMD_READ_IN_PORT = 0xC0,
	KYBRD_CTRL_CMD_READ_OUT_PORT = 0xD0,
	KYBRD_CTRL_CMD_WRITE_OUT_PORT = 0xD1,
	KYBRD_CTRL_CMD_READ_TEST_INPUTS = 0xE0,
	KYBRD_CTRL_CMD_SYSTEM_RESET = 0xFE,
	KYBRD_CTRL_CMD_MOUSE_DISABLE = 0xA7,
	KYBRD_CTRL_CMD_MOUSE_ENABLE = 0xA8,
	KYBRD_CTRL_CMD_MOUSE_PORT_TEST = 0xA9,
	KYBRD_CTRL_CMD_MOUSE_WRITE = 0xD4
};

// scan error codes ------------------------------------------

enum KYBRD_ERROR
{

	KYBRD_ERR_BUF_OVERRUN = 0,
	KYBRD_ERR_ID_RET = 0x83AB,
	KYBRD_ERR_BAT = 0xAA,  //note: can also be L. shift key make code
	KYBRD_ERR_ECHO_RET = 0xEE,
	KYBRD_ERR_ACK = 0xFA,
	KYBRD_ERR_BAT_FAILED = 0xFC,
	KYBRD_ERR_DIAG_FAILED = 0xFD,
	KYBRD_ERR_RESEND_CMD = 0xFE,
	KYBRD_ERR_KEY = 0xFF
};

static char _scancode;
static bool _numlock, _scrolllock, _capslock;
static bool _shift, _alt, _ctrl;

//! original xt scan code set. Array index==make code
//! change what keys the scan code corrospond to if your scan code set is different
static int _kkybrd_scancode_std[] = {

	//! key			scancode
	KEY_UNKNOWN,	 //0
	KEY_ESC,		 //1
	KEY_1,			 //2
	KEY_2,			 //3
	KEY_3,			 //4
	KEY_4,			 //5
	KEY_5,			 //6
	KEY_6,			 //7
	KEY_7,			 //8
	KEY_8,			 //9
	KEY_9,			 //0xa
	KEY_0,			 //0xb
	KEY_MINUS,		 //0xc
	KEY_EQUAL,		 //0xd
	KEY_BACKSPACE,	 //0xe
	KEY_TAB,		 //0xf
	KEY_Q,			 //0x10
	KEY_W,			 //0x11
	KEY_E,			 //0x12
	KEY_R,			 //0x13
	KEY_T,			 //0x14
	KEY_Y,			 //0x15
	KEY_U,			 //0x16
	KEY_I,			 //0x17
	KEY_O,			 //0x18
	KEY_P,			 //0x19
	KEY_LEFTBRACE,	 //0x1a
	KEY_RIGHTBRACE,	 //0x1b
	KEY_ENTER,		 //0x1c
	KEY_LEFTCTRL,	 //0x1d
	KEY_A,			 //0x1e
	KEY_S,			 //0x1f
	KEY_D,			 //0x20
	KEY_F,			 //0x21
	KEY_G,			 //0x22
	KEY_H,			 //0x23
	KEY_J,			 //0x24
	KEY_K,			 //0x25
	KEY_L,			 //0x26
	KEY_SEMICOLON,	 //0x27
	KEY_APOSTROPHE,	 //0x28
	KEY_GRAVE,		 //0x29
	KEY_LEFTSHIFT,	 //0x2a
	KEY_BACKSLASH,	 //0x2b
	KEY_Z,			 //0x2c
	KEY_X,			 //0x2d
	KEY_C,			 //0x2e
	KEY_V,			 //0x2f
	KEY_B,			 //0x30
	KEY_N,			 //0x31
	KEY_M,			 //0x32
	KEY_COMMA,		 //0x33
	KEY_DOT,		 //0x34
	KEY_SLASH,		 //0x35
	KEY_RIGHTSHIFT,	 //0x36
	KEY_KPASTERISK,	 //0x37
	KEY_LEFTALT,	 //0x38
	KEY_SPACE,		 //0x39
	KEY_CAPSLOCK,	 //0x3a
	KEY_F1,			 //0x3b
	KEY_F2,			 //0x3c
	KEY_F3,			 //0x3d
	KEY_F4,			 //0x3e
	KEY_F5,			 //0x3f
	KEY_F6,			 //0x40
	KEY_F7,			 //0x41
	KEY_F8,			 //0x42
	KEY_F9,			 //0x43
	KEY_F10,		 //0x44
	KEY_NUMLOCK,	 //0x45
	KEY_SCROLLLOCK,	 //0x46
	KEY_KP7,		 //0x47
	KEY_KP8,		 //0x48	//keypad up arrow
	KEY_KP9,		 //0x49
	KEY_KPMINUS,	 //0x4a
	KEY_KP4,		 //0x4b // keypad left arrow
	KEY_KP5,		 //0x4c
	KEY_KP6,		 //0x4d // keypad right arrow
	KEY_KPPLUS,		 //0x4e
	KEY_KP1,		 //0x4f
	KEY_KP2,		 //0x50	//keypad down arrow
	KEY_KP3,		 //0x51	//keypad page down
	KEY_KP0,		 //0x52	//keypad insert key
	KEY_KPDOT,		 //0x53	//keypad delete key
	KEY_UNKNOWN,	 //0x54
	KEY_UNKNOWN,	 //0x55
	KEY_UNKNOWN,	 //0x56
	KEY_F11,		 //0x57
	KEY_F12,		 //0x58
};

struct key_event current_kybrd_event;

uint8_t kybrd_ctrl_read_status()
{
	return inportb(KYBRD_CTRL_STATS_REG);
}

void kybrd_ctrl_send_cmd(uint8_t cmd)
{
	while (1)
		if ((kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
			break;

	outportb(KYBRD_CTRL_CMD_REG, cmd);
}

uint8_t kybrd_enc_read_buf()
{
	return inportb(KYBRD_ENC_INPUT_BUF);
}

void kybrd_enc_send_cmd(uint8_t cmd)
{
	while (1)
		if ((kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
			break;

	outportb(KYBRD_ENC_CMD_REG, cmd);
}

void kybrd_set_event_state()
{
	if (_capslock)
		set_bit(0, &current_kybrd_event.state);
	else
		clear_bit(0, &current_kybrd_event.state);

	if (_ctrl)
		set_bit(1, &current_kybrd_event.state);
	else
		clear_bit(1, &current_kybrd_event.state);

	if (_shift)
		set_bit(2, &current_kybrd_event.state);
	else
		clear_bit(2, &current_kybrd_event.state);

	if (_alt)
		set_bit(3, &current_kybrd_event.state);
	else
		clear_bit(3, &current_kybrd_event.state);
}

int32_t i86_kybrd_irq(struct interrupt_registers *regs)
{
	int code = 0;

	//! read scan code only if the kkybrd controller output buffer is full (scan code is in it)
	if (kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_OUT_BUF)
	{
		//! read the scan code
		code = kybrd_enc_read_buf();

		irq_ack(regs->int_no);

		//! is this an extended code? If so, set it and return
		if (code == 0xE0 || code == 0xE1)
			return IRQ_HANDLER_CONTINUE;

		//! test if this is a break code (Original XT Scan Code Set specific)
		if (code & 0x80)
		{  //test bit 7

			//! covert the break code into its make code equivelant
			code -= 0x80;

			//! grab the key
			int key = _kkybrd_scancode_std[code];

			//! test if a special key has been released & set it
			switch (key)
			{
			case KEY_LEFTCTRL:
			case KEY_RIGHTCTRL:
				_ctrl = false;
				break;

			case KEY_LEFTSHIFT:
			case KEY_RIGHTSHIFT:
				_shift = false;
				break;

			case KEY_LEFTALT:
			case KEY_RIGHTALT:
				_alt = false;
				break;
			}

			current_kybrd_event.type = KEY_RELEASE;
			current_kybrd_event.key = key;
			kybrd_set_event_state();
			kybrd_notify_readers(&current_kybrd_event);
		}
		else
		{
			//! this is a make code - set the scan code
			_scancode = code;

			//! grab the key
			int key = _kkybrd_scancode_std[code];

			//! test if user is holding down any special keys & set it
			switch (key)
			{
			case KEY_LEFTCTRL:
			case KEY_RIGHTCTRL:
				_ctrl = true;
				break;

			case KEY_LEFTSHIFT:
			case KEY_RIGHTSHIFT:
				_shift = true;
				break;

			case KEY_LEFTALT:
			case KEY_RIGHTALT:
				_alt = true;
				break;

			case KEY_CAPSLOCK:
				_capslock = (_capslock) ? false : true;
				kkybrd_set_leds(_numlock, _capslock, _scrolllock);
				break;

			case KEY_NUMLOCK:
				_numlock = (_numlock) ? false : true;
				kkybrd_set_leds(_numlock, _capslock, _scrolllock);
				break;

			case KEY_SCROLLLOCK:
				_scrolllock = (_scrolllock) ? false : true;
				kkybrd_set_leds(_numlock, _capslock, _scrolllock);
				break;
			}

			current_kybrd_event.type = KEY_PRRESS;
			current_kybrd_event.key = key;
			kybrd_set_event_state();
			kybrd_notify_readers(&current_kybrd_event);
		}
	}
	else
		irq_ack(regs->int_no);

	return IRQ_HANDLER_CONTINUE;
}

void kkybrd_set_leds(bool num, bool caps, bool scroll)
{
	uint8_t data = 0;

	//! set or clear the bit
	data = (scroll) ? (data | 1) : (data & 1);
	data = (num) ? (num | 2) : (num & 2);
	data = (caps) ? (num | 4) : (num & 4);

	//! send the command -- update keyboard Light Emetting Diods (LEDs)
	kybrd_enc_send_cmd(KYBRD_ENC_CMD_SET_LED);
	kybrd_enc_send_cmd(data);
}

static struct list_head nodelist;
static struct wait_queue_head hwait;

static void kybrd_notify_readers(struct key_event *event)
{
	struct kybrd_inode *iter;
	list_for_each_entry(iter, &nodelist, sibling)
	{
		if (iter->ready == true)
		{
			iter->tail = (iter->tail + 1) % MOUSE_PACKET_QUEUE_LEN;
			if (iter->tail == iter->head)
				iter->head = (iter->head + 1) % MOUSE_PACKET_QUEUE_LEN;
		}
		else
			iter->head = iter->tail;

		iter->packets[iter->tail] = *event;
		iter->ready = true;
	}
	wake_up(&hwait);
}

static int kybrd_open(struct vfs_inode *inode, struct vfs_file *file)
{
	struct kybrd_inode *mi = kcalloc(sizeof(struct kybrd_inode), 1);
	mi->file = file;
	file->private_data = mi;
	list_add_tail(&mi->sibling, &nodelist);

	return 0;
}

static ssize_t kybrd_read(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
	struct kybrd_inode *mi = (struct kybrd_inode *)file->private_data;
	wait_event(&hwait, mi->ready);

	memcpy(buf, &mi->packets[mi->head], sizeof(struct key_event));

	if (mi->tail != mi->head)
		mi->head = (mi->head + 1) % MOUSE_PACKET_QUEUE_LEN;
	else
		mi->ready = false;

	return 0;
}

static unsigned int kybrd_poll(struct vfs_file *file, struct poll_table *pt)
{
	struct kybrd_inode *mi = (struct kybrd_inode *)file->private_data;
	poll_wait(file, &hwait, pt);

	return mi->ready ? (POLLIN | POLLRDNORM) : 0;
}

static int kybrd_release(struct vfs_inode *inode, struct vfs_file *file)
{
	struct kybrd_inode *mi = (struct kybrd_inode *)file->private_data;
	list_del(&mi->sibling);
	kfree(mi);

	return 0;
}

static struct vfs_file_operations kybrd_fops = {
	.open = kybrd_open,
	.read = kybrd_read,
	.poll = kybrd_poll,
	.release = kybrd_release,
};

static struct char_device cdev_kybrd = (struct char_device)DECLARE_CHRDEV("kybrd", KYBRD_MAJOR, 1, 1, &kybrd_fops);

void kkybrd_install()
{
	log("Keyboard: Initializing");
	INIT_LIST_HEAD(&nodelist);
	INIT_LIST_HEAD(&hwait.list);

	log("Keyboard: Mount dev");
	register_chrdev(&cdev_kybrd);
	vfs_mknod("/dev/input/keyboard", S_IFCHR, cdev_kybrd.dev);

	//! Install our interrupt handler (irq 1 uses interrupt 33)
	register_interrupt_handler(IRQ1, i86_kybrd_irq);

	_scancode = 0;

	//! set lock keys and led lights
	_numlock = _scrolllock = _capslock = false;
	kkybrd_set_leds(false, false, false);

	//! shift, ctrl, and alt keys
	_shift = _alt = _ctrl = false;

	log("Keyboard: Done");
}

//============================================================================
//    INTERFACE CLASS BODIES
//============================================================================
//****************************************************************************
//**
//**    END[kybrd.cpp]
//**
//****************************************************************************

#include "kybrd.h"

#include <include/ctype.h>
#include <include/errno.h>
#include <kernel/cpu/hal.h>
#include <kernel/cpu/idt.h>
#include <kernel/devices/kybrd.h>
#include <kernel/devices/mouse.h>
#include <kernel/fs/char_dev.h>
#include <kernel/proc/task.h>
#include <kernel/utils/printf.h>
#include <kernel/utils/string.h>

static void kybrd_notify_readers(struct kybrd_event *event);

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

//! current scan code
static char _scancode;

//! lock keys
static bool _numlock, _scrolllock, _capslock;

//! shift, alt, and ctrl keys current state
static bool _shift, _alt, _ctrl;

//! set if the Basic Assurance Test (BAT) failed
static bool _kkybrd_bat_res = false;

//! set if diagnostics failed
static bool _kkybrd_diag_res = false;

//! set if system should resend last command
static bool _kkybrd_resend_res = false;

//! set if keyboard is disabled
static bool _kkybrd_disable = false;

//! original xt scan code set. Array index==make code
//! change what keys the scan code corrospond to if your scan code set is different
static int _kkybrd_scancode_std[] = {

	//! key			scancode
	KEY_UNKNOWN,	   //0
	KEY_ESCAPE,		   //1
	KEY_1,			   //2
	KEY_2,			   //3
	KEY_3,			   //4
	KEY_4,			   //5
	KEY_5,			   //6
	KEY_6,			   //7
	KEY_7,			   //8
	KEY_8,			   //9
	KEY_9,			   //0xa
	KEY_0,			   //0xb
	KEY_MINUS,		   //0xc
	KEY_EQUAL,		   //0xd
	KEY_BACKSPACE,	   //0xe
	KEY_TAB,		   //0xf
	KEY_Q,			   //0x10
	KEY_W,			   //0x11
	KEY_E,			   //0x12
	KEY_R,			   //0x13
	KEY_T,			   //0x14
	KEY_Y,			   //0x15
	KEY_U,			   //0x16
	KEY_I,			   //0x17
	KEY_O,			   //0x18
	KEY_P,			   //0x19
	KEY_LEFTBRACKET,   //0x1a
	KEY_RIGHTBRACKET,  //0x1b
	KEY_RETURN,		   //0x1c
	KEY_LCTRL,		   //0x1d
	KEY_A,			   //0x1e
	KEY_S,			   //0x1f
	KEY_D,			   //0x20
	KEY_F,			   //0x21
	KEY_G,			   //0x22
	KEY_H,			   //0x23
	KEY_J,			   //0x24
	KEY_K,			   //0x25
	KEY_L,			   //0x26
	KEY_SEMICOLON,	   //0x27
	KEY_QUOTE,		   //0x28
	KEY_GRAVE,		   //0x29
	KEY_LSHIFT,		   //0x2a
	KEY_BACKSLASH,	   //0x2b
	KEY_Z,			   //0x2c
	KEY_X,			   //0x2d
	KEY_C,			   //0x2e
	KEY_V,			   //0x2f
	KEY_B,			   //0x30
	KEY_N,			   //0x31
	KEY_M,			   //0x32
	KEY_COMMA,		   //0x33
	KEY_DOT,		   //0x34
	KEY_SLASH,		   //0x35
	KEY_RSHIFT,		   //0x36
	KEY_KP_ASTERISK,   //0x37
	KEY_RALT,		   //0x38
	KEY_SPACE,		   //0x39
	KEY_CAPSLOCK,	   //0x3a
	KEY_F1,			   //0x3b
	KEY_F2,			   //0x3c
	KEY_F3,			   //0x3d
	KEY_F4,			   //0x3e
	KEY_F5,			   //0x3f
	KEY_F6,			   //0x40
	KEY_F7,			   //0x41
	KEY_F8,			   //0x42
	KEY_F9,			   //0x43
	KEY_F10,		   //0x44
	KEY_KP_NUMLOCK,	   //0x45
	KEY_SCROLLLOCK,	   //0x46
	KEY_HOME,		   //0x47
	KEY_KP_8,		   //0x48	//keypad up arrow
	KEY_PAGEUP,		   //0x49
	KEY_UNKNOWN,	   //0x4a
	KEY_UNKNOWN,	   //0x4b
	KEY_UNKNOWN,	   //0x4c
	KEY_UNKNOWN,	   //0x4d
	KEY_UNKNOWN,	   //0x4e
	KEY_UNKNOWN,	   //0x4f
	KEY_KP_2,		   //0x50	//keypad down arrow
	KEY_KP_3,		   //0x51	//keypad page down
	KEY_KP_0,		   //0x52	//keypad insert key
	KEY_KP_DECIMAL,	   //0x53	//keypad delete key
	KEY_UNKNOWN,	   //0x54
	KEY_UNKNOWN,	   //0x55
	KEY_UNKNOWN,	   //0x56
	KEY_F11,		   //0x57
	KEY_F12,		   //0x58
	KEY_UNKNOWN,	   //0x59
	KEY_UNKNOWN,	   //0x5a
	KEY_LCOMMAND,	   //0x5b
	KEY_RCOMMAND,	   //0x5c
};

//! invalid scan code. Used to indicate the last scan code is not to be reused
const int INVALID_SCANCODE = 0;
struct kybrd_event current_kybrd_event;

uint8_t kybrd_ctrl_read_status();
void kybrd_ctrl_send_cmd(uint8_t);
uint8_t kybrd_enc_read_buf();
void kybrd_enc_send_cmd(uint8_t);

//! read status from keyboard controller
uint8_t kybrd_ctrl_read_status()
{
	return inportb(KYBRD_CTRL_STATS_REG);
}

//! send command byte to keyboard controller
void kybrd_ctrl_send_cmd(uint8_t cmd)
{
	//! wait for kkybrd controller input buffer to be clear
	while (1)
		if ((kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
			break;

	outportb(KYBRD_CTRL_CMD_REG, cmd);
}

//! read keyboard encoder buffer
uint8_t kybrd_enc_read_buf()
{
	return inportb(KYBRD_ENC_INPUT_BUF);
}

//! send command byte to keyboard encoder
void kybrd_enc_send_cmd(uint8_t cmd)
{
	//! wait for kkybrd controller input buffer to be clear
	while (1)
		if ((kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
			break;

	//! send command byte to kybrd encoder
	outportb(KYBRD_ENC_CMD_REG, cmd);
}

//!	keyboard interrupt handler
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
		if (code != 0xE0 && code != 0xE1)
		{
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
				case KEY_LCTRL:
				case KEY_RCTRL:
					_ctrl = false;
					break;

				case KEY_LSHIFT:
				case KEY_RSHIFT:
					_shift = false;
					break;

				case KEY_LALT:
				case KEY_RALT:
					_alt = false;
					break;
				}

				current_kybrd_event.type = KEY_RELEASE;
				current_kybrd_event.key = key;
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
				case KEY_LCTRL:
				case KEY_RCTRL:
					_ctrl = true;
					break;

				case KEY_LSHIFT:
				case KEY_RSHIFT:
					_shift = true;
					break;

				case KEY_LALT:
				case KEY_RALT:
					_alt = true;
					break;

				case KEY_CAPSLOCK:
					_capslock = (_capslock) ? false : true;
					kkybrd_set_leds(_numlock, _capslock, _scrolllock);
					break;

				case KEY_KP_NUMLOCK:
					_numlock = (_numlock) ? false : true;
					kkybrd_set_leds(_numlock, _capslock, _scrolllock);
					break;

				case KEY_SCROLLLOCK:
					_scrolllock = (_scrolllock) ? false : true;
					kkybrd_set_leds(_numlock, _capslock, _scrolllock);
					break;
				}
				// FIXME: MQ 2020-03-22 on Mac 10.15.3, qemu 4.2.0
				// after left/right clicking, next mouse events, first mouse packet state still has left/right state
				// workaround via using left/right command to simuate left/right click
				if (key == KEY_LCOMMAND || key == KEY_RCOMMAND)
				{
					struct mouse_event *mouse = kcalloc(1, sizeof(struct mouse_event));
					mouse->x = 0;
					mouse->y = 0;
					mouse->buttons = key == KEY_LCOMMAND ? MOUSE_LEFT_CLICK : MOUSE_RIGHT_CLICK;
					mouse_notify_readers(mouse);
					kfree(mouse);
				}
				else
				{
					current_kybrd_event.type = KEY_PRRESS;
					current_kybrd_event.key = key;
					kybrd_notify_readers(&current_kybrd_event);
				}
			}
		}

		//! watch for errors
		switch (code)
		{
		case KYBRD_ERR_BAT_FAILED:
			_kkybrd_bat_res = false;
			break;

		case KYBRD_ERR_DIAG_FAILED:
			_kkybrd_diag_res = false;
			break;

		case KYBRD_ERR_RESEND_CMD:
			_kkybrd_resend_res = true;
			break;
		}
	}
	else
		irq_ack(regs->int_no);

	return IRQ_HANDLER_CONTINUE;
}

//! returns scroll lock state
bool kkybrd_get_scroll_lock()
{
	return _scrolllock;
}

//! returns num lock state
bool kkybrd_get_numlock()
{
	return _numlock;
}

//! returns caps lock state
bool kkybrd_get_capslock()
{
	return _capslock;
}

//! returns status of control key
bool kkybrd_get_ctrl()
{
	return _ctrl;
}

//! returns status of alt key
bool kkybrd_get_alt()
{
	return _alt;
}

//! returns status of shift key
bool kkybrd_get_shift()
{
	return _shift;
}

//! tells driver to ignore last resend request
void kkybrd_ignore_resend()
{
	_kkybrd_resend_res = false;
}

//! return if system should redo last commands
bool kkybrd_check_resend()
{
	return _kkybrd_resend_res;
}

//! return diagnostics test result
bool kkybrd_get_diagnostic_res()
{
	return _kkybrd_diag_res;
}

//! return BAT test result
bool kkybrd_get_bat_res()
{
	return _kkybrd_bat_res;
}

//! return last scan code
uint8_t kkybrd_get_last_scan()
{
	return _scancode;
}

//! sets leds
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

//! discards last scan
void kkybrd_discard_last_key()
{
	_scancode = INVALID_SCANCODE;
}

//! convert key to an ascii character
char kkybrd_key_to_ascii(enum KEYCODE code)
{
	uint8_t key = code;

	//! insure key is an ascii character
	if (isascii(key))
	{
		//! if shift key is down or caps lock is on, make the key uppercase
		if (_shift || _capslock)
			if (key >= 'a' && key <= 'z')
				key -= 32;

		if (_shift && !_capslock)
		{
			if (key >= '0' && key <= '9')
				switch (key)
				{
				case '0':
					key = KEY_RIGHTPARENTHESIS;
					break;
				case '1':
					key = KEY_EXCLAMATION;
					break;
				case '2':
					key = KEY_AT;
					break;
				case '3':
					key = KEY_EXCLAMATION;
					break;
				case '4':
					key = KEY_HASH;
					break;
				case '5':
					key = KEY_PERCENT;
					break;
				case '6':
					key = KEY_CARRET;
					break;
				case '7':
					key = KEY_AMPERSAND;
					break;
				case '8':
					key = KEY_ASTERISK;
					break;
				case '9':
					key = KEY_LEFTPARENTHESIS;
					break;
				}
			else
			{
				switch (key)
				{
				case KEY_COMMA:
					key = KEY_LESS;
					break;

				case KEY_DOT:
					key = KEY_GREATER;
					break;

				case KEY_SLASH:
					key = KEY_QUESTION;
					break;

				case KEY_SEMICOLON:
					key = KEY_COLON;
					break;

				case KEY_QUOTE:
					key = KEY_QUOTEDOUBLE;
					break;

				case KEY_LEFTBRACKET:
					key = KEY_LEFTCURL;
					break;

				case KEY_RIGHTBRACKET:
					key = KEY_RIGHTCURL;
					break;

				case KEY_GRAVE:
					key = KEY_TILDE;
					break;

				case KEY_MINUS:
					key = KEY_UNDERSCORE;
					break;

				case KEY_PLUS:
					key = KEY_EQUAL;
					break;

				case KEY_BACKSLASH:
					key = KEY_BAR;
					break;
				}
			}
		}
		//! return the key
		return key;
	}

	//! scan code != a valid ascii char so no convertion is possible
	return 0;
}

//! disables the keyboard
void kkybrd_disable()
{
	kybrd_ctrl_send_cmd(KYBRD_CTRL_CMD_DISABLE);
	_kkybrd_disable = true;
}

//! enables the keyboard
void kkybrd_enable()
{
	kybrd_ctrl_send_cmd(KYBRD_CTRL_CMD_ENABLE);
	_kkybrd_disable = false;
}

//! returns true if keyboard is disabled
bool kkybrd_is_disabled()
{
	return _kkybrd_disable;
}

//! reset the system
void kkybrd_reset_system()
{
	//! writes 11111110 to the output port (sets reset system line low)
	kybrd_ctrl_send_cmd(KYBRD_CTRL_CMD_WRITE_OUT_PORT);
	kybrd_enc_send_cmd(0xfe);
}

//! run self test
bool kkybrd_self_test()
{
	//! send command
	kybrd_ctrl_send_cmd(KYBRD_CTRL_CMD_SELF_TEST);

	//! wait for output buffer to be full
	while (1)
		if (kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_OUT_BUF)
			break;

	//! if output buffer == 0x55, test passed
	return (kybrd_enc_read_buf() == 0x55) ? true : false;
}

struct list_head nodelist;
struct wait_queue_head hwait;

static void kybrd_notify_readers(struct kybrd_event *event)
{
	struct kybrd_inode *iter;
	list_for_each_entry(iter, &nodelist, sibling)
	{
		iter->tail = (iter->tail + 1) / KYBRD_PACKET_QUEUE_LEN;
		iter->packets[iter->tail] = *event;
		if (iter->tail == iter->head)
			iter->head = (iter->head + 1) / KYBRD_PACKET_QUEUE_LEN;
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

	if (mi->head == mi->tail)
		return -EINVAL;

	memcpy(buf, &mi->packets[mi->head], sizeof(int32_t));
	mi->head = (mi->head + 1) % KYBRD_PACKET_QUEUE_LEN;

	if (mi->head == mi->tail)
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

static struct char_device cdev_kybrd = {
	.name = "kybrd",
	.dev = MKDEV(KYBRD_MAJOR, 1),
	.f_ops = &kybrd_fops};

//! prepares driver for use
void kkybrd_install()
{
	DEBUG &&debug_println(DEBUG_INFO, "[keyboard] - Initializing");
	INIT_LIST_HEAD(&nodelist);
	INIT_LIST_HEAD(&hwait.list);

	DEBUG &&debug_println(DEBUG_INFO, "[dev] - Mount kybrd");
	register_chrdev(&cdev_kybrd);
	vfs_mknod("/dev/input/keyboard", S_IFCHR, cdev_kybrd.dev);

	//! Install our interrupt handler (irq 1 uses interrupt 33)
	register_interrupt_handler(IRQ1, i86_kybrd_irq);

	//! assume BAT test is good. If there is a problem, the IRQ handler where catch the error
	_kkybrd_bat_res = true;
	_scancode = 0;

	//! set lock keys and led lights
	_numlock = _scrolllock = _capslock = false;
	kkybrd_set_leds(false, false, false);

	//! shift, ctrl, and alt keys
	_shift = _alt = _ctrl = false;

	DEBUG &&debug_println(DEBUG_INFO, "[keyboard] - Done");
}

//============================================================================
//    INTERFACE CLASS BODIES
//============================================================================
//****************************************************************************
//**
//**    END[kybrd.cpp]
//**
//****************************************************************************

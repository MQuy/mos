#ifndef TERMINAL_ASCI_H
#define TERMINAL_ASCI_H 1

enum
{
	ASCI_SINGLE_SHIFT_TWO = 0,
	ASCI_SINGLE_SHIFT_THREE = 1,
	ASCI_DEVICE_CONTROL_STRING = 2,
	ASCI_CONTROL_SEQUENCE_INTRODUCER = 3,
};

struct asci_sequence
{
	int type;
	void *data;
};

struct asci_control
{
	int type;
	void *data;
};

void asci_init();
int convert_key_event_to_code(unsigned int keycode, unsigned int state, unsigned char *buf);

#endif

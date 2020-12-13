#include <libgui/event.h>
#include <stdlib.h>
#include <string.h>

struct xevent *create_xkey_event(uint32_t key, enum xevent_action action, unsigned int state)
{
	struct xevent *event = calloc(1, sizeof(struct xevent));
	event->type = XKEY_EVENT;

	struct xkey_event *key_event = (struct xkey_event *)event->data;
	key_event->action = action;
	key_event->key = key;
	key_event->state = state;

	return event;
}

struct xevent *create_xmotion_event(int32_t x, int32_t y, unsigned int state)
{
	struct xevent *event = calloc(1, sizeof(struct xevent));
	event->type = XMOTION_EVENT;

	struct xmotion_event *motion_event = (struct xmotion_event *)event->data;
	motion_event->x = x;
	motion_event->y = y;
	motion_event->state = state;

	return event;
}

struct xevent *create_xbutton_event(uint8_t button, enum xevent_action action, int32_t x, int32_t y, unsigned int state)
{
	struct xevent *event = calloc(1, sizeof(struct xevent));
	event->type = XBUTTON_EVENT;

	struct xbutton_event *button_event = (struct xbutton_event *)event->data;
	button_event->action = action;
	button_event->button = button;
	button_event->x = x;
	button_event->y = y;
	button_event->state = state;

	return event;
}

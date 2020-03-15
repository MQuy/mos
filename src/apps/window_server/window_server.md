### Flow

In kenel, running WS as the main user app entry. After switching to WS's kernel space

- create shared memory queue (/dev/shm/ui_events), any ui events will be put in this queue
- save WS's id (get back for scheduling when receiving events like mouse, keyboard)
- map the video buffer into WS userspace (as WS's argument)

In WS's user space

- Initialize ui tree (desktop->window,window)
- Loop
  - get events from /dev/shm/ui_events, no events -> continue
  - get active window, check events belongs to active windown to send events via send_msg
    - get active widget, delegate events to widget's handle
  - render ui tree
  - render mouse

### Data structure

```js
ui_queue {
  ui_event_type type; (mouse move, keypress)
  int xchange, ychange;
  int keychar;
  bool shift, ctrl;
  ui_queue *next;
}

desktop {
  char *vbuf;
  desktop *desktop;
  window *active_window;
  void (*draw)();
}

graphic {
  char *buf;
  int x, y;
  uint width, height;
  void (*signal)(); // signal WS to render
  void (*draw)(); // only be called by WS to fill its buffer
  void (*handle)(ui_event); // handle events from WS
}

window {
  graphic graphic;
  desktop *parent;
  widget *active_widget;
  list_head<ui_event> events;
}

widget {
  graphic graphic;
  window *parent;
  list_head<ui_event> events;
}

label {
  widget widget;
  char *text;
  void (*set_text)();
}

input {
  widget widget;
  char *text;
}

ui_event {
  ui_event_type type;
  list_head<void *on_event> listeners;
}
```

### Libraries

1. Create window (x, y, width, height)

- Create a shared memory (contiguous memory allocation)
- Send frames to WS (start address, length)

2. Create text (window)

- Create a shared memory (contiguous)
- Send frames to WS (start address, length, parent)

3. Create input (window)

- Create a shared memory (contiguous)
- Send frames to WS (start address, length, parent, on_event)

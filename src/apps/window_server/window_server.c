#include <libc/gui/layout.h>
#include <libc/gui/msgui.h>
#include <libc/poll.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>
#include <stdint.h>

#include "src/window_manager.h"

int main(struct framebuffer fb)
{
	int32_t ws_fd = mq_open(WINDOW_SERVER_QUEUE, O_RDONLY, &(struct mq_attr){
															   .mq_msgsize = sizeof(struct msgui),
															   .mq_maxmsg = 32,
														   });
	int32_t mouse_fd = open("/dev/input/mouse", O_RDONLY, 0);
	int32_t krb_fd = open("/dev/input/keyboard", O_RDONLY, 0);

	struct pollfd pfds[3] = {
		{.fd = ws_fd, .events = POLLIN},
		{.fd = mouse_fd, .events = POLLIN},
		{.fd = krb_fd, .events = POLLIN},
	};

	struct msgui ws_buf;
	struct mouse_event mouse_event;
	struct key_event krb_event;

	init_layout(&fb);
	draw_layout();

	while (true)
	{
		int32_t nr = poll(pfds, 3);
		if (nr <= 0)
			continue;

		for (int32_t i = 0; i < 3; ++i)
		{
			if (!(pfds[i].revents & POLLIN))
				continue;

			if (pfds[i].fd == ws_fd)
			{
				memset(&ws_buf, 0, sizeof(struct msgui));
				mq_receive(ws_fd, (char *)&ws_buf, 0, sizeof(struct msgui));

				if (ws_buf.type == MSGUI_WINDOW)
				{
					struct msgui_window *msgwin = (struct msgui_window *)ws_buf.data;
					struct window *win = create_window(msgwin);
					int32_t wfd = mq_open(msgwin->sender, O_WRONLY, &(struct mq_attr){
																		.mq_msgsize = WINDOW_NAME_LENGTH,
																		.mq_maxmsg = 32,
																	});
					mq_send(wfd, win->name, 0, WINDOW_NAME_LENGTH);
					mq_close(wfd);
				}
				else if (ws_buf.type == MSGUI_FOCUS)
				{
					struct msgui_focus *focus = (struct msgui_focus *)ws_buf.data;
					handle_focus_event(focus);
					draw_layout();
				}
			}
			else if (pfds[i].fd == mouse_fd)
			{
				memset(&mouse_event, 0, sizeof(struct mouse_event));
				read(mouse_fd, (char *)&mouse_event, sizeof(struct mouse_event));
				handle_mouse_event(&mouse_event);
				draw_layout();
			}
			else if (pfds[i].fd == krb_fd)
			{
				read(krb_fd, (char *)&krb_event, sizeof(struct key_event));
				handle_keyboard_event(&krb_event);
				draw_layout();
			}
		}
	}

	return 0;
}

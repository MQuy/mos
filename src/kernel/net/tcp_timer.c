#include <kernel/cpu/pit.h>

#include "tcp.h"

void tcp_retransmit_timer(struct timer_list *timer)
{
	struct tcp_sock *tsk = from_timer(tsk, timer, retransmit_timer);

	// TODO: MQ 2020-07-07 Send timeout segment

	mod_timer(&tsk->retransmit_timer, get_current_tick() + 1 * TICKS_PER_SECOND);
}

void tcp_probe_timer(struct timer_list *timer)
{
	struct tcp_sock *tsk = from_timer(tsk, timer, probe_timer);

	// TODO: MQ 2020-07-07 Send probe segment

	del_timer(&tsk->probe_timer);
}

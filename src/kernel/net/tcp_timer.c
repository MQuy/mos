#include "tcp.h"

void tcp_retransmit_timer(struct timer_list *timer)
{
	struct tcp_sock *tsk = from_timer(tsk, timer, retransmit_timer);
}

void tcp_probe_timer(struct timer_list *timer)
{
}

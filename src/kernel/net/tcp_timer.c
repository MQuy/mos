#include <kernel/cpu/pit.h>

#include "tcp.h"

void tcp_retransmit_timer(struct timer_list *timer)
{
	struct tcp_sock *tsk = from_timer(tsk, timer, retransmit_timer);
	struct socket *sock = tsk->inet.sk.sock;

	struct sk_buff *iter;
	list_for_each_entry(iter, &sock->sk->tx_queue, sibling)
	{
		struct tcp_skb_cb *cb = TCP_SKB_CB(iter);
		if (cb->when > get_current_tick() ||
			cb->end_seq >= tsk->snd_nxt ||
			(cb->end_seq - cb->seq + 1) > tcp_sender_available_window(tsk))
			break;
		tcp_send_skb(sock, iter);
	}

	tsk->retransmit_backoff *= 2;
	mod_timer(&tsk->retransmit_timer, get_current_tick() + tsk->retransmit_backoff * TICKS_PER_SECOND);
}

void tcp_persist_timer(struct timer_list *timer)
{
	struct tcp_sock *tsk = from_timer(tsk, timer, persist_timer);
	struct socket *sock = tsk->inet.sk.sock;

	if (!tsk->snd_wnd)
	{
		del_timer(timer);
		tsk->persist_backoff = 1;
		return;
	}

	struct sk_buff *skb = tcp_create_skb(sock,
										 tsk->snd_nxt, tsk->rcv_nxt,
										 TCPCB_FLAG_ACK,
										 NULL, 0,
										 &(uint8_t[]){0}, 1);
	tcp_send_skb(sock, skb);
	tsk->persist_backoff *= 2;
	mod_timer(timer, get_current_tick() + tsk->persist_backoff * TICKS_PER_SECOND);
}

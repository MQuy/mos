#include <kernel/cpu/pit.h>
#include <kernel/proc/task.h>

#include "tcp.h"

void tcp_retransmit_timer(struct timer_list *timer)
{
	struct tcp_sock *tsk = from_timer(tsk, timer, retransmit_timer);
	struct socket *sock = tsk->inet.sk.sock;

	struct sk_buff *skb = list_first_entry_or_null(&sock->sk->tx_queue, struct sk_buff, sibling);
	assert(skb);
	tcp_send_skb(sock, skb);

	// move send head back to begining of tx queue
	sock->sk->send_head = &skb->sibling;

	// count retried syn to re-initialize rto=3
	if (skb->h.tcph->syn)
		tsk->syn_retries++;

	// according to Karn's algorthim, retransmitted segment is not included in RTT measurement
	if (TCP_SKB_CB(skb)->end_seq == tsk->rtt_end_seq && !tsk->rtt_time)
		tsk->rtt_time = 0;

	// after retransmitting 8 times without success we clear srtt and rttvar
	if (tsk->rto >= 128)
	{
		tsk->srtt = 0;
		tsk->rttvar = 0;
	}

	tsk->rto *= 2;
	mod_timer(&tsk->retransmit_timer, get_current_tick() + tsk->rto);
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

void tcp_msl_timer(struct timer_list *timer)
{
	struct tcp_sock *tsk = from_timer(tsk, timer, msl_timer);
	struct socket *sock = tsk->inet.sk.sock;

	del_timer(&tsk->msl_timer);

	assert(tsk->state == TCP_TIME_WAIT || tsk->state == TCP_LAST_ACK);

	tsk->state = TCP_CLOSE;
	tcp_delete_tcb(sock);

	update_thread(sock->sk->owner_thread, THREAD_READY);
}

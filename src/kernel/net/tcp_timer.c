#include <kernel/proc/task.h>
#include <kernel/system/time.h>

#include "tcp.h"

void tcp_retransmit_timer(struct timer_list *timer)
{
	struct tcp_sock *tsk = from_timer(tsk, timer, retransmit_timer);
	struct socket *sock = tsk->inet.sk.sock;

	tsk->rto *= 2;
	debug_println(DEBUG_INFO, "inside timer: %l %l", get_milliseconds(NULL), timer->expires);
	mod_timer(timer, get_milliseconds(NULL) + tsk->rto);

	struct sk_buff *skb = list_first_entry_or_null(&sock->sk->tx_queue, struct sk_buff, sibling);
	assert(skb);
	tcp_send_skb(sock, skb, true);
	debug_println(DEBUG_INFO, "inside timer: %d %d %d %l %l", htonl(skb->h.tcph->sequence_number), tsk->flight_size, tcp_sender_available_window(tsk), TCP_SKB_CB(skb)->when, TCP_SKB_CB(skb)->expires);

	// when the first segment loss occurs
	// -> `tx_queue` cannot be empty (we only clear segment from tx queue when receiving its ack)
	// -> `send_head` cannot be at the beginning of tx queue (otherwise that segment isn't sent yet)
	if (sock->sk->send_head != &skb->sibling)
	{
		tsk->ssthresh = max_t(uint32_t, tsk->flight_size / 2, 2 * tsk->snd_mss);
		tsk->cwnd = (tsk->snd_mss > 2190 ? 2 : (tsk->snd_mss > 1095 ? 3 : 4)) * tsk->snd_mss;
	}

	// move send head back to begining of tx queue
	sock->sk->send_head = &skb->sibling;

	// count retried syn to re-initialize rto=3
	if (skb->h.tcph->syn)
		tsk->syn_retries++;

	// according to Karn's algorthim, retransmitted segment is not included in RTT measurement
	if (tsk->rtt_time)
	{
		assert(TCP_SKB_CB(skb)->end_seq == tsk->rtt_end_seq);
		tsk->rtt_time = 0;
	}

	// after retransmitting 8 times without success we clear srtt and rttvar
	if (tsk->rto >= 128 * 1000)
	{
		tsk->srtt = 0;
		tsk->rttvar = 0;
		// TODO: MQ 2020-07-19 what will we do with the connection after failed 8 times
	}
}

void tcp_persist_timer(struct timer_list *timer)
{
	struct tcp_sock *tsk = from_timer(tsk, timer, persist_timer);
	struct socket *sock = tsk->inet.sk.sock;

	if (tsk->snd_wnd)
	{
		del_timer(timer);
		tsk->persist_backoff = 1000;
		return;
	}

	assert(list_empty(&sock->sk->tx_queue));
	tsk->persist_backoff *= 2;
	mod_timer(timer, get_milliseconds(NULL) + tsk->persist_backoff);

	struct sk_buff *skb = list_first_entry_or_null(&sock->sk->tx_queue, struct sk_buff, sibling);
	assert(skb && tcp_payload_lenth(skb) == 1);
	tcp_send_skb(sock, skb, true);
}

void tcp_msl_timer(struct timer_list *timer)
{
	struct tcp_sock *tsk = from_timer(tsk, timer, msl_timer);
	struct socket *sock = tsk->inet.sk.sock;

	del_timer(timer);

	assert(tsk->state == TCP_TIME_WAIT || tsk->state == TCP_LAST_ACK);
	tsk->state = TCP_CLOSE;
	tcp_delete_tcb(sock);

	update_thread(sock->sk->owner_thread, THREAD_READY);
}

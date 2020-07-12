#include <include/errno.h>
#include <kernel/cpu/pit.h>
#include <kernel/proc/task.h>

#include "tcp.h"

uint32_t tcp_get_option_value(uint8_t *options, void *value, uint8_t len)
{
	if (len == 1)
		*(uint8_t *)value = options[0];
	else if (len == 2)
		*(uint16_t *)value = options[0] + (options[1] << 8);
	else if (len == 4)
		*(uint32_t *)value = options[0] + (options[1] << 8) + (options[2] << 16) + (options[3] << 24);
	else
		return -ERANGE;

	return 0;
}

void tcp_parse_syn_options(uint8_t *options, uint32_t len, uint16_t *rmms, uint8_t *window_scale)
{
	uint32_t opt_rmms;
	uint8_t opt_window_scale;

	for (uint32_t i = 0; i < len;)
	{
		if (options[i] == 0 || options[i] == 1)
		{
			i += 1;
			continue;
		}
		else if (options[i] == 2)
			tcp_get_option_value(&options[i + 2], &opt_rmms, 2);
		else if (options[i] == 3)
			tcp_get_option_value(&options[i + 2], &opt_window_scale, 1);

		i += options[i + 1];
	}

	*rmms = ntohs(opt_rmms);
	*window_scale = opt_window_scale;
}

void tcp_accept_ack(struct socket *sock, uint32_t ack_number, bool is_acked_all)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	tsk->snd_una = ack_number;

	if (tsk->number_of_dup_acks > 0)
		tsk->cwnd = tsk->ssthresh;
	tsk->flight_size = tsk->snd_nxt - tsk->snd_una;
	tsk->number_of_dup_acks = 0;

	struct sk_buff *iter, *next;
	list_for_each_entry_safe(iter, next, &sock->sk->tx_queue, sibling)
	{
		if (TCP_SKB_CB(iter)->end_seq < ack_number || is_acked_all)
			list_del(&iter->sibling);
	}

	struct sk_buff *skb = list_first_entry_or_null(&sock->sk->tx_queue, struct sk_buff, sibling);
	if (skb)
		mod_timer(&tsk->retransmit_timer, TCP_SKB_CB(skb)->expires + tsk->rto);
	else
		del_timer(&tsk->retransmit_timer);

	if (tsk->rtt_time && tsk->rtt_end_seq < ack_number)
	{
		uint32_t rtt = get_current_tick() - tsk->rtt_time;
		tcp_calculate_rto(sock, rtt);
		tsk->rtt_time = 0;
	}
}

bool tcp_is_fin_acked(struct socket *sock)
{
	struct sk_buff *iter;
	list_for_each_entry(iter, &sock->sk->tx_queue, sibling)
	{
		if (iter->h.tcph->fin)
			return false;
	}
	return true;
}

void tcp_handler_close(struct socket *sock, struct sk_buff *skb)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	uint32_t seg_seq = ntohl(skb->h.tcph->sequence_number);
	uint32_t seg_ack = ntohl(skb->h.tcph->ack_number);
	uint16_t payload_len = tcp_payload_lenth(skb);

	if (skb->h.tcph->rst)
		return;

	struct sk_buff *sent_skb;
	if (!skb->h.tcph->ack)
		sent_skb = tcp_create_skb(sock,
								  0, seg_seq + payload_len,
								  TCPCB_FLAG_ACK | TCPCB_FLAG_RST,
								  NULL, 0,
								  NULL, 0);
	else
		sent_skb = tcp_create_skb(sock,
								  seg_ack, tsk->rcv_nxt,
								  TCPCB_FLAG_RST,
								  NULL, 0,
								  NULL, 0);
	tcp_send_skb(sock, sent_skb, false);
}

void tcp_handler_sync(struct socket *sock, struct sk_buff *skb)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	uint32_t ack_number = ntohl(skb->h.tcph->ack_number);
	bool acceptable_ack = false;

	if (skb->h.tcph->ack)
	{
		if (ack_number <= tsk->snd_iss || ack_number > tsk->snd_nxt)
		{
			if (!skb->h.tcph->rst)
			{
				struct sk_buff *skb = tcp_create_skb(sock, ack_number, 0, TCPCB_FLAG_RST, NULL, 0, NULL, 0);
				tcp_send_skb(sock, skb, false);

				update_thread(sock->sk->owner_thread, THREAD_READY);
			}
			return;
		}
		acceptable_ack = tsk->snd_una <= ack_number && ack_number <= tsk->snd_nxt;
	}
	if (skb->h.tcph->rst)
	{
		if (acceptable_ack)
		{
			tcp_enter_close_state(sock);
			tcp_delete_tcb(sock);
		}
		return;
	}
	if (skb->h.tcph->syn && acceptable_ack)
	{
		uint32_t option_len = tcp_option_length(skb);
		if (option_len > 0)
			tcp_parse_syn_options(skb->h.tcph->payload, option_len, &tsk->rcv_mss, &tsk->snd_wds);

		tsk->rcv_irs = ntohl(skb->h.tcph->sequence_number);
		tsk->rcv_nxt = tsk->rcv_irs + 1;

		tsk->snd_nxt = ack_number;
		// according to RFC1323, the window field in SYN (<SYN> or <SYN, ACK>) segment itself is never scaled
		tsk->snd_wnd = ntohs(skb->h.tcph->window);
		tcp_accept_ack(sock, ack_number, false);

		if (tsk->snd_una > tsk->snd_iss)
		{
			tsk->cwnd = (tsk->snd_mss > 2190 ? 2 : (tsk->snd_mss > 1095 ? 3 : 4)) * tsk->snd_mss;
			tsk->ssthresh = ntohl(skb->h.tcph->window);

			struct sk_buff *skb = tcp_create_skb(sock, tsk->snd_nxt, tsk->rcv_nxt, TCPCB_FLAG_ACK, NULL, 0, NULL, 0);
			tcp_send_skb(sock, skb, false);

			update_thread(sock->sk->owner_thread, THREAD_READY);
		}
		else
		{
			struct sk_buff *skb = tcp_create_skb(sock, tsk->snd_iss, tsk->rcv_nxt, TCPCB_FLAG_ACK | TCPCB_FLAG_SYN, NULL, 0, NULL, 0);
			tcp_send_skb(sock, skb, false);
		}
	}
}

void tcp_handler_established(struct socket *sock, struct sk_buff *skb)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	uint32_t seg_seq = ntohl(skb->h.tcph->sequence_number);
	uint32_t seg_ack = ntohl(skb->h.tcph->ack_number);
	uint32_t seg_wnd = ntohs(skb->h.tcph->window) << tsk->snd_wds;
	uint16_t payload_len = tcp_payload_lenth(skb);
	bool acceptable_segment;

	// step one
	if (tsk->rcv_wnd)
	{
		if (payload_len > 0)
			// TODO: MQ 2020-07-06 trimming of any portions that lie outside the window
			acceptable_segment = (tsk->rcv_nxt <= seg_seq && seg_seq < tsk->rcv_nxt + tsk->rcv_wnd) ||
								 (tsk->rcv_nxt <= seg_seq + payload_len && seg_seq + payload_len < tsk->rcv_nxt + tsk->rcv_wnd);
		else
			acceptable_segment = tsk->rcv_nxt <= seg_seq && seg_seq < tsk->rcv_nxt + tsk->rcv_wnd;
	}
	else
	{
		if (payload_len > 0)
			acceptable_segment = false;
		else
			acceptable_segment = ntohl(skb->h.tcph->sequence_number) == tsk->rcv_nxt;
	}

	if (!acceptable_segment)
	{
		if (!skb->h.tcph->rst)
		{
			struct sk_buff *snd_skb = tcp_create_skb(sock, tsk->snd_nxt, tsk->rcv_nxt, TCPCB_FLAG_ACK, NULL, 0, NULL, 0);
			tcp_send_skb(sock, snd_skb, false);
		}
		update_thread(sock->sk->owner_thread, THREAD_READY);
		return;
	}

	// step two
	if (skb->h.tcph->rst)
	{
		if (tsk->state == TCP_SYN_RECV)
		{
			// NOTE: MQ 2020-07-08
			// According to RFC793, we have to handle passive and active OPEN
			// -> to simplify we assume that all cases are active OPEN
			tcp_accept_ack(sock, 0, true);
			tcp_enter_close_state(sock);
			update_thread(sock->sk->owner_thread, THREAD_READY);
			return;
		}
		else if (tsk->state == TCP_ESTABLISHED || tsk->state == TCP_FIN_WAIT1 || tsk->state == TCP_FIN_WAIT2)
		{
			tcp_flush_tx(sock);
			tcp_flush_rx(sock);
			tcp_enter_close_state(sock);
			update_thread(sock->sk->owner_thread, THREAD_READY);
			return;
		}
		else if (tsk->state == TCP_CLOSING || tsk->state == TCP_LAST_ACK || tsk->state == TCP_TIME_WAIT)
		{
			tcp_enter_close_state(sock);
			update_thread(sock->sk->owner_thread, THREAD_READY);
			return;
		}
	}

	// step fourth
	if (skb->h.tcph->syn &&
		tsk->state == TCP_SYN_RECV && tsk->state == TCP_ESTABLISHED &&
		tsk->state == TCP_FIN_WAIT1 && tsk->state == TCP_FIN_WAIT2 &&
		tsk->state == TCP_CLOSE_WAIT && tsk->state == TCP_CLOSING &&
		tsk->state == TCP_LAST_ACK && tsk->state == TCP_TIME_WAIT)
	{
		struct sk_buff *snd_skb = tcp_create_skb(sock, seg_ack, tsk->rcv_nxt, TCPCB_FLAG_RST, NULL, 0, NULL, 0);
		tcp_send_skb(sock, snd_skb, false);

		tcp_flush_tx(sock);
		tcp_flush_rx(sock);
		tcp_enter_close_state(sock);
		update_thread(sock->sk->owner_thread, THREAD_READY);
		return;
	}

	// step fifth
	if (!skb->h.tcph->ack)
		return;

	if (tsk->state == TCP_SYN_RECV)
	{
		if (tsk->snd_una < seg_ack && seg_ack <= tsk->snd_nxt)
			tsk->state = TCP_ESTABLISHED;
		else if (!acceptable_segment)
		{
			struct sk_buff *snd_skb = tcp_create_skb(sock, tsk->snd_nxt, tsk->rcv_nxt, TCPCB_FLAG_RST, NULL, 0, NULL, 0);
			tcp_send_skb(sock, snd_skb, false);
		}
	}
	else if (tsk->state == TCP_ESTABLISHED || tsk->state == TCP_CLOSE_WAIT ||
			 tsk->state == TCP_FIN_WAIT1 || tsk->state == TCP_FIN_WAIT2 ||
			 tsk->state == TCP_CLOSING)
	{
		if (tsk->snd_una < seg_ack && seg_ack <= tsk->snd_nxt)
		{
			tcp_calculate_congestion(sock, seg_ack);
			tcp_accept_ack(sock, seg_ack, false);

			if (tsk->snd_wl1 < seg_seq || (tsk->snd_wl1 == seg_seq && tsk->snd_wl2 <= seg_ack))
			{
				tsk->snd_wnd = seg_wnd;
				tsk->snd_wl1 = seg_seq;
				tsk->snd_wl2 = seg_ack;
			}
		}
		// if sequence number doesn't change -> no new data being sent
		// if ack number doesn't change -> no new incoming data being acked
		// different window size comparing the curent one and empty segment
		// -> window update segment
		else if (tsk->snd_una == seg_ack && tsk->rcv_nxt == seg_seq && tsk->snd_wnd != seg_wnd && payload_len == 0)
		{
			tsk->snd_wnd = seg_wnd;
		}
		// fast retransmit and fast recovery
		else if (tsk->flight_size > 0 &&
				 !skb->h.tcph->syn && !skb->h.tcph->fin &&
				 tsk->snd_una == seg_ack &&
				 seg_wnd > 0)
		{
			// should be combined with above, but I am skeptical
			if (payload_len != 0 || tsk->snd_wnd != seg_wnd)
				debug_println(DEBUG_WARNING, "fast retransmit: duplicated ack is in wrong state");

			tsk->number_of_dup_acks++;

			if (tsk->number_of_dup_acks == 3)
			{
				tsk->ssthresh = max_t(uint16_t, tsk->flight_size / 2, 2 * tsk->snd_mss);
				tsk->cwnd = tsk->ssthresh + 3 * tsk->snd_mss;

				struct sk_buff *skb = list_first_entry_or_null(&sock->sk->tx_queue, struct sk_buff, sibling);
				assert(skb);
				tcp_send_skb(sock, skb, true);
			}
			else if (tsk->number_of_dup_acks > 3)
				tsk->cwnd += tsk->snd_mss;
		}
		else if (seg_ack > tsk->snd_nxt)
		{
			struct sk_buff *snd_skb = tcp_create_skb(sock, tsk->snd_nxt, tsk->rcv_nxt, TCPCB_FLAG_ACK, NULL, 0, NULL, 0);
			tcp_send_skb(sock, snd_skb, false);
			return;
		}
	}
	if (tsk->state == TCP_FIN_WAIT1)
	{
		if (tcp_is_fin_acked(sock))
			tsk->state = TCP_FIN_WAIT2;
	}
	else if (tsk->state == TCP_CLOSING)
	{
		if (tcp_is_fin_acked(sock))
			tsk->state = TCP_TIME_WAIT;
	}
	else if (tsk->state == TCP_LAST_ACK)
	{
		if (tcp_is_fin_acked(sock))
		{
			tcp_enter_close_state(sock);
			update_thread(sock->sk->owner_thread, THREAD_READY);
			return;
		}
	}
	else if (tsk->state == TCP_TIME_WAIT)
	{
		struct sk_buff *snd_skb = tcp_create_skb(sock, tsk->snd_nxt, tsk->rcv_nxt, TCPCB_FLAG_ACK, NULL, 0, NULL, 0);
		tcp_send_skb(sock, snd_skb, false);
	}

	// step seventh
	bool is_sent_ack = false;
	if (tsk->state == TCP_ESTABLISHED || tsk->state == TCP_FIN_WAIT1 || tsk->state == TCP_FIN_WAIT2)
	{
		if (payload_len > 0 || skb->h.tcph->push)
			list_add_tail(&skb->sibling, &sock->sk->rx_queue);

		if (payload_len > 0)
		{
			tsk->rcv_nxt += payload_len;
			// TODO: MQ 2020-07-06 congestion and timer
			struct sk_buff *snd_skb = tcp_create_skb(sock, tsk->snd_nxt, tsk->rcv_nxt, TCPCB_FLAG_ACK, NULL, 0, NULL, 0);
			tcp_send_skb(sock, snd_skb, false);
			is_sent_ack = true;
		}
	}

	// step eighth
	if (skb->h.tcph->fin)
	{
		if (tsk->state == TCP_CLOSE || tsk->state == TCP_LISTEN || tsk->state == TCP_SYN_SENT)
			return;

		if (!is_sent_ack)
		{
			tsk->rcv_nxt += 1;
			struct sk_buff *snd_skb = tcp_create_skb(sock, tsk->snd_nxt, tsk->rcv_nxt, TCPCB_FLAG_ACK, NULL, 0, NULL, 0);
			tcp_send_skb(sock, snd_skb, false);
		}

		if (tsk->state == TCP_SYN_RECV || tsk->state == TCP_ESTABLISHED)
			tsk->state = TCP_CLOSE_WAIT;
		else if (tsk->state == TCP_FIN_WAIT1)
		{
			if (tcp_is_fin_acked(sock))
			{
				tsk->state = TCP_TIME_WAIT;
				mod_timer(&tsk->msl_timer, get_current_tick() + MAX_SEGMENT_LIFETIME * 2);
				del_timer(&tsk->retransmit_timer);
				del_timer(&tsk->persist_timer);
			}
			else
				tsk->state = TCP_CLOSING;
		}
		else if (tsk->state == TCP_FIN_WAIT2)
		{
			tsk->state = TCP_TIME_WAIT;
			mod_timer(&tsk->msl_timer, get_current_tick() + MAX_SEGMENT_LIFETIME * 2);
			del_timer(&tsk->retransmit_timer);
			del_timer(&tsk->persist_timer);
		}
		else if (tsk->state == TCP_TIME_WAIT)
		{
			mod_timer(&tsk->msl_timer, get_current_tick() + MAX_SEGMENT_LIFETIME * 2);
		}
	}

	update_thread(sock->sk->owner_thread, THREAD_READY);
}

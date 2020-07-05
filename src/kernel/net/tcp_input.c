#include <include/errno.h>

#include "tcp.h"

uint32_t tcp_option_get_value(uint8_t *options, uint32_t *value, uint8_t len)
{
	if (len == 1)
		*value = options[0];
	else if (len == 2)
		*value = options[0] + (options[1] << 8);
	else if (len == 3)
		*value = options[0] + (options[1] << 8) + (options[2] << 16);
	else if (len == 4)
		*value = options[0] + (options[1] << 8) + (options[2] << 16) + (options[3] << 24);
	else
		return -ERANGE;

	return 0;
}

void tcp_options_parse_syn(uint8_t *options, uint32_t len, uint16_t *rmms)
{
	uint32_t opt_rmms;

	for (uint32_t i = 0; i < len;)
	{
		if (options[i] == 0 || options[i] == 1)
		{
			i += 1;
			continue;
		}
		else if (options[i] == 2)
			tcp_option_get_value(&options[i + 2], &opt_rmms, 2);

		i += options[i + 1];
	}

	*rmms = ntohs(opt_rmms);
}

void tcp_retransmision_queue_acked(struct socket *sock, uint32_t ack_number)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	struct sk_buff *iter, *next;
	list_for_each_entry_safe(iter, next, &sock->sk->tx_queue, sibling)
	{
		struct tcp_skb_cb *cb = (struct tcp_skb_cb *)iter->cb;
		if (cb->end_seq < ack_number)
		{
			list_del(&iter->sibling);
			tsk->snd_wnd += tcp_payload_lenth(iter);
		}
	}

	struct sk_buff *skb = list_first_entry(&sock->sk->tx_queue, struct sk_buff, sibling);
	mod_timer(&tsk->retransmit_timer, TCP_SKB_CB(skb)->when + tsk->rto);
}

void tcp_handler_sync_sent(struct socket *sock, struct sk_buff *skb)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	uint32_t ack_number = ntohl(skb->h.tcph->ack_number);
	bool acceptable_ack;

	if (skb->h.tcph->ack)
	{
		if (ack_number <= tsk->snd_iss || ack_number > tsk->snd_nxt)
		{
			if (!skb->h.tcph->rst)
			{
				struct sk_buff *skb = tcp_create_skb(sock, ack_number, 0, TCPCB_FLAG_RST, NULL, 0, NULL, 0);
				tcp_transmit_skb(sock, skb);
			}
			return;
		}
		acceptable_ack = tsk->snd_una <= ack_number && ack_number <= tsk->snd_nxt;
	}
	if (skb->h.tcph->rst)
	{
		if (acceptable_ack)
			tcp_enter_close_state(sock);
		return;
	}
	if (skb->h.tcph->syn && acceptable_ack)
	{
		tsk->rcv_irs = ntohl(skb->h.tcph->sequence_number);
		tsk->rcv_nxt = tsk->rcv_irs + 1;
		tsk->rcv_wnd = ntohl(skb->h.tcph->window);

		tsk->snd_una = ack_number;
		tsk->snd_nxt = ack_number;
		tcp_retransmision_queue_acked(sock, ack_number);

		uint32_t option_len = tcp_option_length(skb);
		if (option_len > 0)
			tcp_options_parse_syn(skb->h.tcph->payload, option_len, &tsk->rcv_mss);

		if (tsk->snd_una > tsk->snd_iss)
		{
			tsk->cwnd = 3 * tsk->snd_mss;
			tsk->ssthresh = ntohl(skb->h.tcph->window);

			struct sk_buff *skb = tcp_create_skb(sock, tsk->snd_nxt, tsk->rcv_nxt, TCPCB_FLAG_ACK, NULL, 0, NULL, 0);
			tcp_transmit_skb(sock, skb);
		}
		else
		{
			struct sk_buff *skb = tcp_create_skb(sock, tsk->snd_iss, tsk->rcv_nxt, TCPCB_FLAG_ACK | TCPCB_FLAG_SYN, NULL, 0, NULL, 0);
			tcp_transmit_skb(sock, skb);
			// TODO: MQ 2020-07-06 Redo the syn flow
		}
	}
}

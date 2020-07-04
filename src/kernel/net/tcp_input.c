#include <kernel/net/sk_buff.h>

#include "tcp.h"

void tcp_retransmision_queue_acked(struct socket *sock, uint32_t ack_number)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	struct sk_buff *iter, *next;
	list_for_each_entry_safe(iter, next, &sock->sk->tx_queue, sibling)
	{
		struct tcp_skb_cb *cb = (struct tcp_skb_cb *)iter->cb;
		if (cb->end_seq < ack_number)
			list_del(&iter->sibling);
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
		tsk->snd_una = ack_number;
		tsk->snd_nxt = ack_number;

		tcp_retransmision_queue_acked(sock, ack_number);

		if (tsk->snd_una > tsk->snd_iss)
		{
			struct sk_buff *skb = tcp_create_skb(sock, tsk->snd_nxt, tsk->rcv_nxt, TCPCB_FLAG_ACK, NULL, 0, NULL, 0);
			tcp_transmit_skb(sock, skb);
		}
		else
		{
			struct sk_buff *skb = tcp_create_skb(sock, tsk->snd_iss, tsk->rcv_nxt, TCPCB_FLAG_ACK | TCPCB_FLAG_SYN, NULL, 0, NULL, 0);
			tcp_transmit_skb(sock, skb);
		}
	}
}

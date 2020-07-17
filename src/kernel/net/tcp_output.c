#include <kernel/net/neighbour.h>
#include <kernel/proc/task.h>
#include <kernel/system/time.h>
#include <kernel/utils/math.h>
#include <kernel/utils/string.h>

#include "tcp.h"

extern volatile struct thread *current_thread;

struct sk_buff *tcp_create_skb(struct socket *sock,
							   uint32_t sequence_number, uint32_t ack_number,
							   uint16_t flags,
							   void *options, uint16_t option_len,
							   void *payload, uint16_t payload_len)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	struct sk_buff *skb = skb_alloc(MAX_TCP_HEADER + option_len, payload_len);
	skb->dev = tsk->inet.sk.dev;

	skb_put(skb, payload_len);
	memcpy(skb->data, payload, payload_len);

	skb_push(skb, sizeof(struct tcp_packet) + option_len);
	skb->h.tcph = (struct tcp_packet *)skb->data;
	tcp_build_header(skb->h.tcph,
					 tsk->inet.ssin.sin_addr, tsk->inet.ssin.sin_port,
					 tsk->inet.dsin.sin_addr, tsk->inet.dsin.sin_port,
					 sequence_number,
					 ack_number,
					 flags,
					 tsk->rcv_wnd,
					 options, option_len,
					 skb->len);

	skb_push(skb, sizeof(struct ip4_packet));
	skb->nh.iph = (struct ip4_packet *)skb->data;
	ip4_build_header(skb->nh.iph, skb->len, IP4_PROTOCAL_TCP, tsk->inet.ssin.sin_addr, tsk->inet.dsin.sin_addr, rand());

	skb_push(skb, sizeof(struct ethernet_packet));
	skb->mac.eh = (struct ethernet_packet *)skb->data;
	uint8_t *dest_mac = lookup_mac_addr_for_ethernet(skb->dev, tsk->inet.dsin.sin_addr);
	ethernet_build_header(skb->mac.eh, ETH_P_IP, skb->dev->dev_addr, dest_mac);

	struct tcp_skb_cb *cb = (struct tcp_skb_cb *)skb->cb;
	cb->seq = sequence_number;
	// payload_len counts both lower and upper boundary
	cb->end_seq = sequence_number + max(0, (int)payload_len - 1);
	cb->flags = flags;

	return skb;
}

void tcp_send_skb(struct socket *sock, struct sk_buff *skb, bool is_retransmitted)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	struct tcp_skb_cb *cb = (struct tcp_skb_cb *)skb->cb;
	uint16_t payload_len = tcp_payload_lenth(skb);

	tsk->flight_size += payload_len;
	// we increase snd nxt only if data, syn or fin segment (ghost segment)
	if (!is_retransmitted && (payload_len > 0 || skb->h.tcph->syn || skb->h.tcph->fin))
		tsk->snd_nxt = cb->end_seq + 1;

	cb->when = get_milliseconds(NULL);
	cb->expires = cb->when + tsk->rto;

	tcp_state_transition(sock, cb->flags);

	if (skb->h.tcph->syn)
		mod_timer(&tsk->retransmit_timer, cb->expires);

	ethernet_sendmsg(skb);
}

void tcp_transmit(struct socket *sock)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);

	while (!list_empty(&sock->sk->tx_queue))
	{
		while (tcp_sender_available_window(tsk) > 0 && sock->sk->send_head)
		{
			struct sk_buff *skb = list_entry(sock->sk->send_head, struct sk_buff, sibling);
			tcp_send_skb(sock, skb, false);

			// according to rfc6298, kick off only one RTT measurement at the time
			// the segment has to be the first element in tx queue
			if (!tsk->rtt_time)
			{
				assert(&skb->sibling == sock->sk->tx_queue.next);
				struct tcp_skb_cb *cb = TCP_SKB_CB(skb);
				tsk->rtt_end_seq = cb->end_seq;
				tsk->rtt_time = cb->when;
			}

			sock->sk->send_head = sock->sk->send_head->next;
			if (sock->sk->send_head == &sock->sk->tx_queue)
				sock->sk->send_head = NULL;
		}
		update_thread(current_thread, THREAD_WAITING);
		schedule();
	}
};

void tcp_tx_queue_add_skb(struct socket *sock, struct sk_buff *skb)
{
	if (!sock->sk->send_head)
		sock->sk->send_head = &skb->sibling;
	list_add_tail(&skb->sibling, &sock->sk->tx_queue);
}

void tcp_transmit_skb(struct socket *sock, struct sk_buff *skb)
{
	tcp_tx_queue_add_skb(sock, skb);
	tcp_transmit(sock);
}

#include <kernel/cpu/pit.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/ip.h>
#include <kernel/net/neighbour.h>
#include <kernel/net/sk_buff.h>
#include <kernel/proc/task.h>
#include <kernel/utils/math.h>
#include <kernel/utils/printf.h>
#include <kernel/utils/string.h>

#include "tcp.h"

#define MAX_TCP_HEADER (sizeof(struct ethernet_packet) + sizeof(struct ip4_packet) + sizeof(struct tcp_packet))

extern volatile struct thread *current_thread;

struct sk_buff *tcp_create_skb(struct socket *sock, uint32_t sequence_number, uint32_t ack_number, uint8_t flags, uint8_t *options, uint32_t option_len, uint8_t *payload, uint32_t payload_len)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	struct sk_buff *skb = skb_alloc(MAX_TCP_HEADER + option_len, payload_len);
	skb->dev = tsk->inet.sk.dev;

	memcpy(skb->data, payload, payload_len);

	skb_push(skb, sizeof(struct tcp_packet) + option_len);
	skb->h.tcph = (struct tcp_packet *)skb->data;
	tcp_build_header(skb->h.tcph,
					 tsk->inet.ssin.sin_addr, tsk->inet.ssin.sin_port,
					 tsk->inet.dsin.sin_addr, tsk->inet.dsin.sin_port,
					 sequence_number,
					 ack_number,
					 flags,
					 tsk->snd_wnd,
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
	cb->end_seq = sequence_number + payload_len;
	cb->flags = flags;

	return skb;
}

void tcp_send_skb(struct socket *sock, struct sk_buff *skb)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	struct tcp_skb_cb *cb = (struct tcp_skb_cb *)skb->cb;

	tsk->snd_nxt = cb->end_seq + 1;
	tsk->snd_wnd -= skb->len;

	cb->when = get_current_tick() + tsk->rto;
	tcp_state_transition(sock, cb->flags);

	if (cb->when < tsk->retransmit_timer.expires)
		mod_timer(&tsk->retransmit_timer, cb->when);

	ethernet_sendmsg(skb);
}

void tcp_transmit(struct socket *sock)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);

	while (sock->sk->send_head)
	{
		while (tsk->snd_wnd > 0 && tsk->rcv_wnd > 0 && sock->sk->send_head)
		{
			struct sk_buff *skb = list_entry(sock->sk->send_head, struct sk_buff, sibling);
			tcp_send_skb(sock, skb);

			sock->sk->send_head = sock->sk->send_head->next;
			if (sock->sk->send_head == &sock->sk->tx_queue)
				sock->sk->send_head = NULL;
		}
		if (tsk->rcv_wnd == 0)
		{
			// TODO: MQ 2020-07-04 schedule probe timer
		}
		update_thread(current_thread, THREAD_WAITING);
		schedule();
	}
};

void tcp_transmit_skb(struct socket *sock, struct sk_buff *skb)
{
	if (!sock->sk->send_head)
		sock->sk->send_head = &skb->sibling;
	list_add_tail(&skb->sibling, &sock->sk->tx_queue);
	tcp_transmit(sock);
}

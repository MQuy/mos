#include "tcp.h"

#include <include/errno.h>
#include <kernel/cpu/pit.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include <kernel/utils/math.h>
#include <kernel/utils/string.h>

extern volatile struct thread *current_thread;

uint16_t tcp_calculate_checksum(struct tcp_packet *tcp, uint16_t tcp_len, uint32_t source_ip, uint32_t dest_ip)
{
	tcp->checksum = 0;
	return transport_calculate_checksum(tcp, tcp_len, IP4_PROTOCAL_TCP, source_ip, dest_ip);
}

int tcp_validate_header(struct tcp_packet *tcp, uint16_t tcp_len, uint32_t source_ip, uint32_t dest_ip)
{
	uint16_t received_checksum = tcp->checksum;
	uint16_t packet_checksum = tcp_calculate_checksum(tcp, tcp_len, source_ip, dest_ip);
	tcp->checksum = received_checksum;

	return packet_checksum != received_checksum ? -EPROTO : 0;
}

uint8_t *tcp_set_option_value(uint8_t *options, uint8_t code, uint8_t len, void *value)
{
	options[0] = code;
	// According to RFC793, tcp option length includes option-kind and option-length
	options[1] = len + 2;
	memcpy(options + 2, value, len);

	return options + 2 + len;
}

void tcp_build_syn_options(struct socket *sock, uint8_t **options, uint32_t *len)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	*options = kcalloc(1, MAX_OPTION_LEN);
	uint8_t *iter = *options;

	iter = tcp_set_option_value(iter, 2, 2, &(uint16_t[]){htons(tsk->snd_mss)});
	iter = tcp_set_option_value(iter, 3, 1, &(uint8_t[]){0});

	*len = WORD_ALIGN(iter - *options);
}

void tcp_build_header(struct tcp_packet *tcp,
					  uint32_t source_ip, uint16_t source_port,
					  uint32_t dest_ip, uint16_t dest_port,
					  uint32_t seq_number, uint32_t ack_number,
					  uint16_t flags,
					  uint16_t window,
					  void *options, uint32_t option_len, uint32_t packet_len)
{
	assert(option_len % 4 == 0);

	tcp->source_port = htons(source_port);
	tcp->dest_port = htons(dest_port);
	tcp->sequence_number = htonl(seq_number);
	tcp->ack_number = htonl(ack_number);
	tcp->data_offset = (sizeof(struct tcp_packet) + option_len) / 4;
	tcp->urg = (flags & TCPCB_FLAG_URG) != 0;
	tcp->ack = (flags & TCPCB_FLAG_ACK) != 0;
	tcp->push = (flags & TCPCB_FLAG_PSH) != 0;
	tcp->rst = (flags & TCPCB_FLAG_RST) != 0;
	tcp->syn = (flags & TCPCB_FLAG_SYN) != 0;
	tcp->fin = (flags & TCPCB_FLAG_FIN) != 0;
	tcp->window = htons(window);
	tcp->checksum = 0;
	tcp->urgent_pointer = 0;
	memcpy(tcp->payload, options, option_len);
	// NOTE: MQ 2020-07-05 tcp data has to be filled to be able to calculate checksum
	tcp->checksum = tcp_calculate_checksum(tcp, packet_len, source_ip, dest_ip);
}

void tcp_create_tcb(struct tcp_sock *tsk)
{
	tsk->state = TCP_CLOSE;

	// The liberal or optimistic position
	tsk->snd_mss = round(ETH_DATA_LEN - (sizeof(struct ip4_packet) + sizeof(struct tcp_packet)), 4);
	tsk->snd_iss = 0;
	tsk->snd_una = tsk->snd_iss;
	tsk->snd_nxt = tsk->snd_una + 1;
	tsk->snd_wl1 = 0;
	tsk->snd_wl2 = 0;
	tsk->snd_wnd = ETH_MAX_MTU;
	tsk->snd_wds = 0;
	tsk->rcv_mss = tsk->snd_mss;
	tsk->rcv_irs = 0;
	tsk->rcv_nxt = 0;
	tsk->rcv_wnd = ETH_MAX_MTU;

	tsk->ssthresh = ETH_MAX_MTU;
	tsk->cwnd = tsk->snd_mss;

	tsk->rto = 1 * TICKS_PER_SECOND;

	// NOTE: MQ 2020-07-9
	// retransmit and probe timers are embedded (not pointers)
	// make sure in correct state -> clear connections in those
	list_del(&tsk->retransmit_timer.sibling);
	list_del(&tsk->persist_timer.sibling);
	tsk->retransmit_backoff = 1;
	tsk->retransmit_timer = (struct timer_list)TIMER_INITIALIZER(tcp_retransmit_timer, UINT32_MAX);
	tsk->persist_backoff = 1;
	tsk->persist_timer = (struct timer_list)TIMER_INITIALIZER(tcp_persist_timer, UINT32_MAX);
}

void tcp_delete_tcb(struct socket *sock)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	memset((uint8_t *)tsk + sizeof(struct inet_sock), 0, sizeof(struct tcp_sock) - sizeof(struct inet_sock));
}

void tcp_enter_close_state(struct socket *sock)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);

	tsk->state = TCP_CLOSE;
	del_timer(&tsk->retransmit_timer);
	del_timer(&tsk->retransmit_timer);
}

void tcp_flush_tx(struct socket *sock)
{
	struct sk_buff *iter, *next;
	list_for_each_entry_safe(iter, next, &sock->sk->tx_queue, sibling)
	{
		list_del(&iter->sibling);
		skb_free(iter);
	}
	sock->sk->send_head = NULL;
}

void tcp_flush_rx(struct socket *sock)
{
	struct sk_buff *iter, *next;
	list_for_each_entry_safe(iter, next, &sock->sk->rx_queue, sibling)
	{
		list_del(&iter->sibling);
		skb_free(iter);
	}
}

void tcp_state_transition(struct socket *sock, uint8_t flags)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);

	switch (tsk->state)
	{
	case TCP_CLOSE:
		if (flags == TCPCB_FLAG_SYN)
			tsk->state = TCP_SYN_SENT;
		break;
	case TCP_LISTEN:
		if (flags == TCPCB_FLAG_SYN)
			tsk->state = TCP_SYN_SENT;
		break;
	case TCP_SYN_SENT:
		if (flags == (TCPCB_FLAG_ACK | TCPCB_FLAG_SYN))
			tsk->state = TCP_SYN_RECV;
		else if (flags == TCPCB_FLAG_ACK)
			tsk->state = TCP_ESTABLISHED;
		else if (flags == TCPCB_FLAG_RST)
			tsk->state = TCP_CLOSE;
		break;
	case TCP_ESTABLISHED:
		if (flags & TCPCB_FLAG_FIN)
			tsk->state = TCP_FIN_WAIT1;
		break;
	case TCP_FIN_WAIT2:
		if (flags & TCPCB_FLAG_ACK)
			tsk->state = TCP_TIME_WAIT;
		break;
	}
}

int tcp_return_code(struct socket *sock, int success_code)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);

	switch (tsk->state)
	{
	case TCP_ESTABLISHED:
	case TCP_FIN_WAIT1:
		return success_code;

	default:
		return -1;
	}
}

int tcp_bind(struct socket *sock, struct sockaddr *myaddr, int sockaddr_len)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	memcpy(&tsk->inet.ssin, myaddr, sockaddr_len);

	return 0;
}

int tcp_listen(struct socket *sock, int backlog)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	tcp_create_tcb(tsk);

	// TODO: MQ 2020-07-09

	tsk->state = TCP_LISTEN;
	return 0;
}

int tcp_accept(struct socket *sock, struct sockaddr *addr, int sockaddr_len)
{
	// TODO: MQ 2020-07-09
	return 0;
}

int tcp_connect(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len)
{
	sock->state = SS_CONNECTING;

	struct tcp_sock *tsk = tcp_sk(sock->sk);
	memcpy(&tsk->inet.dsin, vaddr, sockaddr_len);

	tcp_create_tcb(tsk);
	uint32_t sequence_number = rand();
	tsk->snd_iss = sequence_number;

	uint8_t *options;
	uint32_t option_len;
	tcp_build_syn_options(sock, &options, &option_len);
	struct sk_buff *skb = tcp_create_skb(sock, sequence_number, 0, TCPCB_FLAG_SYN, options, option_len, NULL, 0);
	kfree(options);

	tcp_transmit_skb(sock, skb);
	sock->state = SS_CONNECTED;
	return tcp_return_code(sock, 0);
}

int tcp_sendmsg(struct socket *sock, void *msg, size_t msg_len)
{
	if (sock->state == SS_DISCONNECTED)
		return -ESHUTDOWN;

	struct tcp_sock *tsk = tcp_sk(sock->sk);
	uint16_t mmss = min(tsk->snd_mss, tsk->rcv_mss);
	size_t msg_sent_len = 0;
	for (; msg_sent_len < msg_len && tsk->state == TCP_ESTABLISHED;)
	{
		uint16_t mwnd;
		// receiving window = 0, sleep till get the signal from probe timer to contine (wnd > 0)
		while (true)
		{
			mwnd = min_t(uint16_t, min(tsk->cwnd, tcp_sender_available_window(tsk)), msg_len);
			if (mwnd > 0)
				break;
			update_thread(current_thread, THREAD_WAITING);
			schedule();
		}
		for (uint16_t ibatch = 0; ibatch < mwnd;)
		{
			uint32_t accumulated_sent_len = msg_sent_len + ibatch;
			uint16_t advertised_window = min_t(uint16_t, min(mmss, mwnd), msg_len - accumulated_sent_len);
			uint16_t flags = accumulated_sent_len + advertised_window == msg_len ? TCPCB_FLAG_PSH : 0;
			assert(accumulated_sent_len + advertised_window <= msg_len);
			struct sk_buff *skb = tcp_create_skb(sock,
												 tsk->snd_nxt + accumulated_sent_len, tsk->rcv_nxt,
												 TCPCB_FLAG_ACK | flags,
												 NULL, 0,
												 (uint8_t *)msg + accumulated_sent_len, advertised_window);
			tcp_tx_queue_add_skb(sock, skb);
			ibatch += advertised_window;
		}

		msg_sent_len += mwnd;
		tcp_transmit(sock);
	}

	return tcp_return_code(sock, msg_sent_len);
}

int tcp_recvmsg(struct socket *sock, void *msg, size_t msg_len)
{
	if (sock->state == SS_DISCONNECTED)
		return -ESHUTDOWN;

	struct tcp_sock *tsk = tcp_sk(sock->sk);
	sock->sk->rx_length = msg_len;

	struct sk_buff *last_skb = NULL;
	while (tsk->state == TCP_ESTABLISHED || tsk->state == TCP_FIN_WAIT1)
	{
		uint16_t received_len = 0;
		struct sk_buff *iter;
		list_for_each_entry(iter, &sock->sk->rx_queue, sibling)
		{
			received_len += tcp_payload_lenth(iter);
			assert(received_len <= msg_len);

			if (msg_len == received_len || iter->h.tcph->push)
			{
				last_skb = iter;
				break;
			}
		}
		if (last_skb)
			break;

		update_thread(current_thread, THREAD_WAITING);
		schedule();
	}

	uint32_t received_len = 0;
	struct sk_buff *iter, *next;
	list_for_each_entry_safe(iter, next, &sock->sk->rx_queue, sibling)
	{
		list_del(&iter->sibling);

		uint16_t payload_len = tcp_payload_lenth(iter);
		memcpy(msg + received_len, tcp_payload(iter), payload_len);
		received_len += payload_len;

		if (iter == last_skb)
			break;
	}

	sock->sk->rx_length = 0;
	return tcp_return_code(sock, received_len);
}

int tcp_shutdown(struct socket *sock)
{
	if (sock->state == SS_DISCONNECTED)
		return -ESHUTDOWN;

	struct tcp_sock *tsk = tcp_sk(sock->sk);
	struct sk_buff *skb = tcp_create_skb(sock,
										 tsk->snd_nxt, tsk->rcv_nxt,
										 TCPCB_FLAG_ACK | TCPCB_FLAG_FIN,
										 NULL, 0,
										 NULL, 0);
	tcp_transmit_skb(sock, skb);

	while (tsk->state != TCP_CLOSE)
	{
		update_thread(current_thread, THREAD_WAITING);
		schedule();
	}

	return 0;
}

int tcp_handler(struct socket *sock, struct sk_buff *skb)
{
	if (sock->state == SS_DISCONNECTED)
		return -ESHUTDOWN;

	struct tcp_sock *tsk = tcp_sk(sock->sk);
	int32_t ret = ethernet_rcv(skb);
	if (ret < 0)
		return ret;

	ret = ip4_rcv(skb);
	if (ret < 0)
		return ret;

	if (skb->nh.iph->protocal != IP4_PROTOCAL_TCP)
		return -EPROTO;

	struct tcp_packet *tcp = (struct tcp_packet *)skb->data;
	int tcp_len = ntohs(skb->nh.iph->total_length) - sizeof(struct ip4_packet);
	ret = tcp_validate_header(tcp, tcp_len, ntohl(skb->nh.iph->source_ip), ntohl(skb->nh.iph->dest_ip));
	if (ret < 0)
		return ret;

	if (tsk->inet.ssin.sin_addr == ntohl(skb->nh.iph->dest_ip) && tsk->inet.ssin.sin_port == ntohs(tcp->dest_port) &&
		tsk->inet.dsin.sin_addr == ntohl(skb->nh.iph->source_ip) && tsk->inet.dsin.sin_port == ntohs(tcp->source_port))
	{
		skb->h.tcph = (struct tcp_packet *)skb->data;

		// switch branch for state
		switch (tsk->state)
		{
		case TCP_CLOSE:
			tcp_handler_close(sock, skb);
			break;
		case TCP_SYN_SENT:
			tcp_handler_sync(sock, skb);
			break;
		case TCP_SYN_RECV:
		case TCP_FIN_WAIT1:
		case TCP_FIN_WAIT2:
		case TCP_CLOSE_WAIT:
		case TCP_CLOSING:
		case TCP_LAST_ACK:
		case TCP_TIME_WAIT:
		case TCP_ESTABLISHED:
			if (tsk->snd_wnd && !skb->h.tcph->window)
				mod_timer(&tsk->persist_timer, get_current_tick() + 1 * TICKS_PER_SECOND);
			tcp_handler_established(sock, skb);
			break;
		}
	}
	return 0;
}

struct proto_ops tcp_proto_ops = {
	.family = PF_INET,
	.obj_size = sizeof(struct tcp_sock),
	.bind = tcp_bind,
	.listen = tcp_listen,
	.accept = tcp_accept,
	.connect = tcp_connect,
	.sendmsg = tcp_sendmsg,
	.recvmsg = tcp_recvmsg,
	.shutdown = tcp_shutdown,
	.handler = tcp_handler,
};

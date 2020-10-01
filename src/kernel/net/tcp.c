#include "tcp.h"

#include <memory/vmm.h>
#include <proc/task.h>
#include <shared/errno.h>
#include <system/time.h>
#include <utils/math.h>
#include <utils/string.h>

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

	del_timer(&tsk->msl_timer);
	tsk->msl_timer = (struct timer_list)TIMER_INITIALIZER(tcp_msl_timer, UINT32_MAX);

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
	tsk->flight_size = 0;
	tsk->number_of_dup_acks = 0;

	// NOTE: MQ 2020-07-09
	// retransmit and probe timers are embedded (not pointers)
	// clear them from timer queue to make sure in correct state for later initialization
	list_del(&tsk->retransmit_timer.sibling);
	list_del(&tsk->persist_timer.sibling);
	tsk->rto = 1000;
	tsk->retransmit_timer = (struct timer_list)TIMER_INITIALIZER(tcp_retransmit_timer, UINT32_MAX);
	tsk->persist_backoff = 1000;
	tsk->persist_timer = (struct timer_list)TIMER_INITIALIZER(tcp_persist_timer, UINT32_MAX);

	tsk->rtt_end_seq = 0;
	tsk->rtt_time = 0;
	tsk->syn_retries = 0;
}

void tcp_delete_tcb(struct socket *sock)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);

	del_timer(&tsk->msl_timer);

	tsk->rto = 1000;
	del_timer(&tsk->retransmit_timer);
	tsk->persist_backoff = 1000;
	del_timer(&tsk->persist_timer);

	tsk->rtt_end_seq = 0;
	tsk->rtt_time = 0;
	tsk->syn_retries = 0;
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
	enum tcp_state prev_state = tsk->state;

	switch (prev_state)
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
	case TCP_CLOSE_WAIT:
		if (flags & TCPCB_FLAG_FIN)
			tsk->state = TCP_LAST_ACK;
		break;
	}

	// after three-way handshake if there is a retransmited syn -> rto=3 and cwnd=smss
	if (prev_state != tsk->state && tsk->state == TCP_ESTABLISHED && tsk->syn_retries > 0)
	{
		tsk->rto = 3;
		tsk->cwnd = tsk->snd_mss;
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

void tcp_calculate_rto(struct socket *sock, uint32_t rtt)
{
#define K 4
#define G 100
#define beta 0.25
#define alpha 0.125

	struct tcp_sock *tsk = tcp_sk(sock->sk);

	if (!tsk->srtt)
	{
		tsk->srtt = rtt;
		tsk->rttvar = rtt / 2;
	}
	else
	{
		tsk->rttvar = (1 - beta) * tsk->rttvar + beta * abs((long)tsk->srtt - (long)rtt);
		tsk->srtt = (1 - alpha) * tsk->srtt + alpha * rtt;
	}
	tsk->rto = max_t(uint32_t, tsk->srtt + max_t(uint32_t, G, K * tsk->rttvar), 1000);
}

void tcp_calculate_congestion(struct socket *sock, uint32_t seg_ack)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);

	if (tsk->cwnd <= tsk->ssthresh)
		tsk->cwnd += min(seg_ack - tsk->snd_una, tsk->snd_mss);
	else
		tsk->cwnd += tsk->snd_mss * tsk->snd_mss / tsk->cwnd;

	// assume that cwnd never exceed maximum mtu
	tsk->cwnd = min_t(uint32_t, tsk->cwnd, ETH_MAX_MTU);
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
	assert(false);

	tsk->state = TCP_LISTEN;
	return 0;
}

int tcp_accept(struct socket *sock, struct sockaddr *addr, int sockaddr_len)
{
	// TODO: MQ 2020-07-09
	assert(false);

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
	uint32_t mmss = min(tsk->snd_mss, tsk->rcv_mss);
	size_t msg_sent_len = 0;
	for (uint32_t starting_nxt = tsk->snd_nxt; msg_sent_len < msg_len && tsk->state == TCP_ESTABLISHED;)
	{
		uint32_t mwnd = min(min(tsk->cwnd, tcp_sender_available_window(tsk)), msg_len - msg_sent_len);
		// if mwnd==0 <- cwnd canot be zero, msg_len - msg_sent_len cannot be zero
		// -> the only case mwnd=0 is snd.wnd=0
		if (mwnd == 0)
		{
			assert(tsk->snd_una == tsk->snd_nxt);
			assert(!tsk->snd_wnd);
			uint16_t flags = msg_sent_len + 1 == msg_len ? TCPCB_FLAG_PSH : 0;
			struct sk_buff *skb = tcp_create_skb(sock,
												 tsk->snd_nxt, tsk->rcv_nxt,
												 flags | TCPCB_FLAG_ACK,
												 NULL, 0,
												 (uint8_t *)msg + msg_sent_len, 1);

			mod_timer(&tsk->persist_timer, get_milliseconds(NULL) + 1000);
			tcp_transmit_skb(sock, skb);
			del_timer(&tsk->persist_timer);
		}

		for (uint32_t iwnd = 0; iwnd < mwnd;)
		{
			uint32_t advertised_window = min(min(mmss, mwnd), msg_len - msg_sent_len);
			uint16_t flags = msg_sent_len + advertised_window == msg_len ? TCPCB_FLAG_PSH : 0;
			assert(msg_sent_len + advertised_window <= msg_len);
			struct sk_buff *skb = tcp_create_skb(sock,
												 starting_nxt + msg_sent_len, tsk->rcv_nxt,
												 TCPCB_FLAG_ACK | flags,
												 NULL, 0,
												 (uint8_t *)msg + msg_sent_len, advertised_window);
			tcp_tx_queue_add_skb(sock, skb);
			iwnd += advertised_window;
			msg_sent_len += advertised_window;
		}

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
	.ioctl = inet_ioctl,
	.listen = tcp_listen,
	.accept = tcp_accept,
	.connect = tcp_connect,
	.sendmsg = tcp_sendmsg,
	.recvmsg = tcp_recvmsg,
	.shutdown = tcp_shutdown,
	.handler = tcp_handler,
};

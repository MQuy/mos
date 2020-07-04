#include "tcp.h"

#include <include/errno.h>
#include <kernel/cpu/pit.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/ip.h>
#include <kernel/net/sk_buff.h>
#include <kernel/utils/math.h>
#include <kernel/utils/printf.h>
#include <kernel/utils/string.h>

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

void tcp_build_header(struct tcp_packet *tcp,
					  uint32_t source_ip, uint16_t source_port,
					  uint32_t dest_ip, uint16_t dest_port,
					  uint32_t seq_number, uint32_t ack_number,
					  uint16_t flags,
					  uint16_t window,
					  uint8_t *options, uint32_t option_len, uint32_t packet_len)
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

void tcp_sock_init(struct tcp_sock *tsk, uint32_t sequence_number)
{
	tsk->state = TCP_CLOSE;

	tsk->snd_iss = sequence_number;
	tsk->snd_una = tsk->snd_iss;
	tsk->snd_nxt = tsk->snd_una + 1;
	tsk->snd_wnd = ETH_DATA_LEN;
	tsk->rcv_irs = 0;
	tsk->rcv_nxt = 0;
	tsk->rcv_wnd = ETH_DATA_LEN;

	tsk->ssthresh = ETH_MAX_MTU;
	tsk->cwnd = ETH_DATA_LEN;

	tsk->rto = 1 * TICKS_PER_SECOND;
	tsk->retransmit_timer = (struct timer_list)TIMER_INITIALIZER(tcp_retransmit_timer, UINT32_MAX);
	tsk->probe_timer = (struct timer_list)TIMER_INITIALIZER(tcp_retransmit_timer, UINT32_MAX);
}

void tcp_enter_close_state(struct socket *sock)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);

	tsk->state = TCP_CLOSE;
	mod_timer(&tsk->retransmit_timer, UINT32_MAX);
	mod_timer(&tsk->probe_timer, UINT32_MAX);
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
	case TCP_SYN_SENT:
		if (flags == (TCPCB_FLAG_ACK | TCPCB_FLAG_SYN))
			tsk->state = TCP_SYN_RECV;
		else if (flags == TCPCB_FLAG_ACK)
			tsk->state = TCP_ESTABLISHED;
	}
}

int tcp_bind(struct socket *sock, struct sockaddr *myaddr, int sockaddr_len)
{
	struct tcp_sock *tsk = tcp_sk(sock->sk);
	memcpy(&tsk->inet.ssin, myaddr, sockaddr_len);
	return 0;
}

int tcp_connect(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len)
{
	sock->state = SS_CONNECTING;

	struct tcp_sock *tsk = tcp_sk(sock->sk);
	memcpy(&tsk->inet.dsin, vaddr, sockaddr_len);

	uint32_t sequence_number = rand();
	tcp_sock_init(tsk, sequence_number);

	struct sk_buff *skb = tcp_create_skb(sock, sequence_number, 0, TCPCB_FLAG_SYN, NULL, 0, NULL, 0);
	tcp_transmit_skb(sock, skb);

	sock->state = SS_CONNECTED;
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

		if (!tsk->rcv_wnd && ntohs(skb->h.tcph->window))
			mod_timer(&tsk->probe_timer, UINT32_MAX);

		// switch branch for state
		switch (tsk->state)
		{
		case TCP_SYN_SENT:
			tcp_handler_sync_sent(sock, skb);
			break;
		}
	}
	return 0;
}

struct proto_ops tcp_proto_ops = {
	.family = PF_INET,
	.obj_size = sizeof(struct tcp_sock),
	.bind = tcp_bind,
	.connect = tcp_connect,
	// .sendmsg = tcp_sendmsg,
	// .recvmsg = tcp_recvmsg,
	// .shutdown = tcp_shutdown,
	.handler = tcp_handler,
};

#ifndef NET_TCP_H
#define NET_TCP_H

#include <kernel/net/ethernet.h>
#include <kernel/net/ip.h>
#include <kernel/net/net.h>
#include <kernel/net/sk_buff.h>
#include <kernel/system/timer.h>
#include <kernel/utils/printf.h>
#include <stdint.h>

#define MAX_OPTION_LEN 40
#define MAX_TCP_HEADER (sizeof(struct ethernet_packet) + sizeof(struct ip4_packet) + sizeof(struct tcp_packet))

#define TCPCB_FLAG_FIN 0x01
#define TCPCB_FLAG_SYN 0x02
#define TCPCB_FLAG_RST 0x04
#define TCPCB_FLAG_PSH 0x08
#define TCPCB_FLAG_ACK 0x10
#define TCPCB_FLAG_URG 0x20
#define TCPCB_FLAG_ECE 0x40
#define TCPCB_FLAG_CWR 0x80

enum tcp_state
{
	TCP_ESTABLISHED = 1,
	TCP_SYN_SENT,
	TCP_SYN_RECV,
	TCP_FIN_WAIT1,
	TCP_FIN_WAIT2,
	TCP_TIME_WAIT,
	TCP_CLOSE,
	TCP_CLOSE_WAIT,
	TCP_LAST_ACK,
	TCP_LISTEN,
	TCP_CLOSING, /* now a valid state */
};

struct tcp_skb_cb
{
	uint32_t seq;
	uint32_t end_seq;
	uint8_t flags;
	unsigned long when;
};

struct tcp_sock
{
	struct inet_sock inet;
	enum tcp_state state;

	// sender sequence variables
	uint16_t snd_mss;
	uint32_t snd_iss;
	uint32_t snd_una;
	uint32_t snd_nxt;
	uint32_t snd_wnd;

	// receiver sequence variables
	uint16_t rcv_mss;
	uint32_t rcv_irs;
	uint32_t rcv_nxt;
	uint32_t rcv_wnd;

	// congestion
	uint32_t ssthresh;
	uint32_t cwnd;

	// timer
	uint32_t rto;
	uint32_t srtt;
	uint32_t rttvar;
	struct timer_list retransmit_timer;
	struct timer_list probe_timer;
};

struct __attribute__((packed)) tcp_packet
{
	uint16_t source_port;
	uint16_t dest_port;
	uint32_t sequence_number;
	uint32_t ack_number;
	uint8_t ns : 1;
	uint8_t reserved : 3;
	uint8_t data_offset : 4;
	uint8_t fin : 1;
	uint8_t syn : 1;
	uint8_t rst : 1;
	uint8_t push : 1;
	uint8_t ack : 1;
	uint8_t urg : 1;
	uint8_t ece : 1;
	uint8_t cwr : 1;
	uint16_t window;
	uint16_t checksum;
	uint16_t urgent_pointer;
	uint8_t payload[];
};

#define TCP_SKB_CB(__skb) ((struct tcp_skb_cb *)&((__skb)->cb[0]))

static inline struct tcp_sock *tcp_sk(struct sock *sk)
{
	return (struct tcp_sock *)sk;
}

static inline uint16_t tcp_option_length(struct sk_buff *skb)
{
	assert(skb->h.tcph);
	return skb->h.tcph->data_offset * 4 - sizeof(struct tcp_packet);
}

static inline uint16_t tcp_payload_lenth(struct sk_buff *skb)
{
	assert(skb->nh.iph);
	assert(skb->h.tcph);
	return ntohs(skb->nh.iph->total_length) - (skb->nh.iph->ihl + skb->h.tcph->data_offset) * 4;
}

void tcp_build_header(struct tcp_packet *tcp,
					  uint32_t source_ip, uint16_t source_port,
					  uint32_t dest_ip, uint16_t dest_port,
					  uint32_t seq_number, uint32_t ack_number,
					  uint16_t flags,
					  uint16_t window,
					  uint8_t *options, uint32_t option_len, uint32_t packet_len);
struct sk_buff *tcp_create_skb(struct socket *sock,
							   uint32_t sequence_number, uint32_t ack_number,
							   uint8_t flags,
							   uint8_t *options, uint32_t option_len,
							   uint8_t *payload, uint32_t payload_len);
void tcp_transmit(struct socket *sock);
void tcp_transmit_skb(struct socket *sock, struct sk_buff *skb);
void tcp_handler_sync_sent(struct socket *sock, struct sk_buff *skb);
void tcp_retransmit_timer(struct timer_list *timer);
void tcp_probe_timer(struct timer_list *timer);
void tcp_enter_close_state(struct socket *sock);
void tcp_state_transition(struct socket *sock, uint8_t flags);

#endif

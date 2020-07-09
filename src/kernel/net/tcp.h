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
	uint16_t flags;
	unsigned long expires;
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
	uint32_t snd_wl1;
	uint32_t snd_wl2;
	uint8_t snd_wds;  // window scale, only support incomming

	// receiver sequence variables
	uint16_t rcv_mss;
	uint32_t rcv_irs;
	uint32_t rcv_nxt;
	uint16_t rcv_wnd;

	// congestion
	uint16_t ssthresh;
	uint16_t cwnd;

	// timer
	uint32_t rto;
	struct timer_list retransmit_timer;
	uint32_t srtt;
	uint32_t rttvar;

	struct timer_list persist_timer;
	uint16_t persist_backoff;

	// rtt
	uint32_t rtt_end_seq;
	uint32_t rtt_time;
	uint8_t syn_retries;
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

static inline uint8_t *tcp_payload(struct sk_buff *skb)
{
	assert(skb->h.tcph);
	return skb->h.tcph->payload + tcp_option_length(skb);
}

static inline uint16_t tcp_payload_lenth(struct sk_buff *skb)
{
	assert(skb->nh.iph);
	assert(skb->h.tcph);

	int payload_len = ntohs(skb->nh.iph->total_length) - (skb->nh.iph->ihl + skb->h.tcph->data_offset) * 4;
	assert(payload_len >= 0);
	return (uint16_t)payload_len;
}

static inline uint16_t tcp_sender_available_window(struct tcp_sock *tsk)
{
	return tsk->snd_una + tsk->snd_wnd - tsk->snd_nxt;
}

void tcp_build_header(struct tcp_packet *tcp,
					  uint32_t source_ip, uint16_t source_port,
					  uint32_t dest_ip, uint16_t dest_port,
					  uint32_t seq_number, uint32_t ack_number,
					  uint16_t flags,
					  uint16_t window,
					  void *options, uint32_t option_len,
					  uint32_t packet_len);
struct sk_buff *tcp_create_skb(struct socket *sock,
							   uint32_t sequence_number, uint32_t ack_number,
							   uint16_t flags,
							   void *options, uint16_t option_len,
							   void *payload, uint16_t payload_len);
void tcp_transmit(struct socket *sock);
void tcp_transmit_skb(struct socket *sock, struct sk_buff *skb);
void tcp_tx_queue_add_skb(struct socket *sock, struct sk_buff *skb);
void tcp_send_skb(struct socket *sock, struct sk_buff *skb);
void tcp_handler_close(struct socket *sock, struct sk_buff *skb);
void tcp_handler_sync(struct socket *sock, struct sk_buff *skb);
void tcp_handler_established(struct socket *sock, struct sk_buff *skb);
void tcp_retransmit_timer(struct timer_list *timer);
void tcp_persist_timer(struct timer_list *timer);
void tcp_enter_close_state(struct socket *sock);
void tcp_delete_tcb(struct socket *sock);
void tcp_state_transition(struct socket *sock, uint8_t flags);
void tcp_flush_tx(struct socket *sock);
void tcp_flush_rx(struct socket *sock);
void tcp_calculate_rto(struct socket *sock, uint32_t rtt);

#endif

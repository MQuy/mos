#include <arpa/inet.h>
#include <errno.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define CHECKSUM_MASK 0xFFFF

enum
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
	TCP_CLOSING /* now a valid state */
};
struct __attribute__((packed)) ip4_pseudo_header
{
	uint32_t source_ip;
	uint32_t dest_ip;
	uint8_t zeros;
	uint8_t protocal;
	uint16_t transport_length;
};

uint32_t packet_checksum_start(void *packet, uint16_t size)
{
	uint32_t checksum = 0;

	uint16_t ibytes = size;
	uint16_t *chunk = (uint16_t *)packet;
	while (ibytes > 1)
	{
		checksum += *chunk;

		ibytes -= 2;
		chunk += 1;
	}
	if (ibytes == 1)
		checksum += *(uint8_t *)chunk;

	while (checksum > CHECKSUM_MASK)
		checksum = (checksum & CHECKSUM_MASK) + (checksum >> 16);

	return checksum;
}

uint16_t singular_checksum(void *packet, uint16_t size)
{
	uint32_t checksum = packet_checksum_start(packet, size);
	return ~checksum & CHECKSUM_MASK;
}

uint16_t transport_calculate_checksum(void *segment, uint16_t segment_len, uint8_t protocal, uint32_t source_ip, uint32_t dest_ip)
{
	struct ip4_pseudo_header *ip4_pseudo_header = malloc(sizeof(struct ip4_pseudo_header));
	ip4_pseudo_header->source_ip = htonl(source_ip);
	ip4_pseudo_header->dest_ip = htonl(dest_ip);
	ip4_pseudo_header->zeros = 0;
	ip4_pseudo_header->protocal = protocal;
	ip4_pseudo_header->transport_length = htons(segment_len);

	uint32_t ip4_checksum_start = packet_checksum_start(ip4_pseudo_header, sizeof(struct ip4_pseudo_header));
	uint32_t udp_checksum_start = packet_checksum_start(segment, segment_len);
	uint32_t checksum = ip4_checksum_start + udp_checksum_start;

	while (checksum > CHECKSUM_MASK)
		checksum = (checksum & CHECKSUM_MASK) + (checksum >> 16);

	free(ip4_pseudo_header);
	return ~checksum & CHECKSUM_MASK;
}

void build_tcp_header(int server_fd,
					  void *msg,
					  uint32_t source_addr, uint32_t source_port,
					  uint32_t dst_addr, uint32_t dst_port,
					  uint32_t seq, uint32_t ack_seq,
					  uint16_t window,
					  uint16_t flags)
{
	struct tcphdr *stcp = (struct tcphdr *)msg;
	stcp->source = htons(source_port);
	stcp->dest = htons(dst_port);
	stcp->seq = htonl(seq);
	stcp->ack_seq = htonl(ack_seq);
	stcp->doff = 5;
	stcp->urg = (flags & TCP_FLAG_URG) != 0;
	stcp->ack = (flags & TCP_FLAG_ACK) != 0;
	stcp->psh = (flags & TCP_FLAG_PSH) != 0;
	stcp->rst = (flags & TCP_FLAG_RST) != 0;
	stcp->syn = (flags & TCP_FLAG_SYN) != 0;
	stcp->fin = (flags & TCP_FLAG_FIN) != 0;
	stcp->window = htons(window);
	stcp->check = 0;
	stcp->check = transport_calculate_checksum(stcp,
											   sizeof(struct tcphdr),
											   IPPROTO_TCP,
											   source_addr,
											   dst_addr);
}

int main(int argc, char *argv[])
{
	int server_fd;
	struct sockaddr_in server_addr, client_addr;
	int source_port = strtol(argv[1], NULL, 0);
	int dst_port = strtol(argv[2], NULL, 0);

	server_fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);

	if (server_fd < 0)
		perror("socket error");

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(source_port);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		perror("bind failed");

	char rmsg[2000];
	char smsg[2000];
	int tcp_state = TCP_CLOSE;
	int counter = 0;
	uint32_t snd_una, snd_nxt;
	uint32_t rcv_nxt;
	uint16_t rcv_wnd = 4096;

	while (recv(server_fd, rmsg, sizeof(rmsg), 0) >= 0)
	{
		struct iphdr *rip = (struct iphdr *)rmsg;
		struct tcphdr *rtcp = (struct tcphdr *)(rmsg + rip->ihl * 4);

		if (htons(rtcp->source) != dst_port && htons(rtcp->dest) != source_port)
			continue;

		memset(smsg, 0, sizeof(smsg));

		client_addr.sin_family = AF_INET;
		client_addr.sin_addr.s_addr = rip->saddr;
		client_addr.sin_port = rtcp->source;

		if (tcp_state == TCP_CLOSE && rtcp->syn)
		{
			snd_una = snd_nxt = rand();
			rcv_nxt = htonl(rtcp->seq) + 1;
			build_tcp_header(server_fd,
							 smsg,
							 htonl(rip->daddr), source_port,
							 htonl(rip->saddr), dst_port,
							 snd_nxt, rcv_nxt, rcv_wnd, TCP_FLAG_SYN | TCP_FLAG_ACK);
			sendto(server_fd, smsg, sizeof(struct tcphdr), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
			tcp_state = TCP_SYN_RECV;
		}
		else if (tcp_state == TCP_SYN_RECV && rtcp->ack)
		{
			snd_una = snd_nxt = htonl(rtcp->ack_seq);
			rcv_nxt = htonl(rtcp->seq);
			tcp_state = TCP_ESTABLISHED;
		}
		else if (tcp_state == TCP_ESTABLISHED)
		{
			counter++;
			int payload_len = htons(rip->tot_len) - rip->ihl * 4 - rtcp->doff * 4;
			uint16_t flags = 0;
			uint32_t seg_seq = htonl(rtcp->seq);
			uint32_t seg_ack = htonl(rtcp->ack_seq);
			uint32_t seg_wnd = htonl(rtcp->window);

			if (2 == counter)
				continue;

			snd_una = seg_ack;
			if (rcv_nxt == seg_seq)
			{
				if (seg_seq + payload_len > rcv_nxt)
					rcv_nxt = seg_seq + payload_len;

				if (rtcp->fin)
				{
					flags = TCP_FLAG_FIN;
					tcp_state = TCP_LAST_ACK;
					rcv_nxt += 1;
				}
			}

			build_tcp_header(server_fd,
							 smsg,
							 htonl(rip->daddr), source_port,
							 htonl(rip->saddr), dst_port,
							 snd_nxt, rcv_nxt,
							 rcv_wnd,
							 TCP_FLAG_ACK | flags);
			sendto(server_fd, smsg, sizeof(struct tcphdr), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
		}
		else if (tcp_state == TCP_LAST_ACK && rtcp->ack)
		{
			tcp_state = TCP_CLOSE;
			puts("tcp close");
		}
		memset(rmsg, 0, sizeof(rmsg));
	}
	close(server_fd);

	return 0;
}

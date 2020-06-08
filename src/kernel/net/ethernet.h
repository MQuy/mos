#ifndef NET_ETHERNET_H
#define NET_ETHERNET_H

#include <stdint.h>

struct sk_buff;

struct __attribute__((packed)) ethernet_packet
{
	uint8_t dest_mac[6];
	uint8_t source_mac[6];
	uint16_t type;
	uint8_t payload[];
};

void ethernet_build_header(struct ethernet_packet *packet, uint16_t protocal, uint8_t *source_mac, uint8_t *dest_mac);
void ethernet_sendmsg(struct sk_buff *skb);
int ethernet_rcv(struct sk_buff *skb);

#endif

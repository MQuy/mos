#ifndef NET_ICMP_H
#define NET_ICMP_H

#include <stdint.h>

#define ICMP_ECHO 0
#define ICMP_REPLY 0
#define ICMP_REQUEST 8

struct sk_buff;

struct __attribute__((packed)) icmp_packet
{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	union
	{
		struct
		{
			uint16_t id;
			uint16_t sequence;
		} echo;
		struct
		{
			uint16_t _unused;
			uint16_t mtu;
		} frag;
		uint32_t rest_of_header;
	} un;
	uint8_t payload[];
};

int icmp_validate_packet(struct icmp_packet *icmp);
struct icmp_packet *icmp_create_packet(struct icmp_packet *icmp,
									   uint8_t type,
									   uint32_t rest_of_header,
									   uint8_t *payload,
									   uint32_t payload_len);
void icmp_reply(uint32_t source_ip,
				uint8_t *dest_mac, uint32_t dest_ip,
				uint32_t identification,
				uint32_t rest_of_header,
				uint8_t *payload, uint32_t payload_len);
int icmp_rcv(struct sk_buff *skb);
void ping(uint32_t dest_ip);

#endif

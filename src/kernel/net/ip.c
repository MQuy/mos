#include "ip.h"

#include <include/errno.h>
#include <include/if_ether.h>
#include <memory/vmm.h>
#include <net/ethernet.h>
#include <net/neighbour.h>
#include <net/net.h>
#include <net/sk_buff.h>
#include <utils/math.h>

#define IP4_TTL 0x80

int ip4_validate_header(struct ip4_packet *ip, uint8_t protocal)
{
	int ret = 0;
	uint16_t received_checksum = ip->header_checksum;

	ip->header_checksum = 0;
	uint16_t packet_checksum = singular_checksum(ip, sizeof(struct ip4_packet));

	// TODO: MQ 2020-06-10 Support 5 < ihl <=15 (max)
	if (ip->version != 4 || ip->ihl != 5 || ip->protocal != protocal || received_checksum != packet_checksum)
		ret = -EPROTO;

	ip->header_checksum = received_checksum;
	return ret;
}

struct ip4_packet *ip4_build_header(struct ip4_packet *packet, uint16_t packet_size, uint8_t protocal, uint32_t source_ip, uint32_t dest_ip, uint32_t identification)
{
	packet->version = 4;
	packet->ihl = 5;
	packet->type_of_service = 0;
	packet->total_length = htons(packet_size);
	packet->identification = htonl(identification);
	packet->flags = 0;
	packet->fragment_offset = 0;
	packet->time_to_live = IP4_TTL;
	packet->protocal = protocal;
	packet->source_ip = htonl(source_ip);
	packet->dest_ip = htonl(dest_ip);
	packet->header_checksum = singular_checksum(packet, sizeof(struct ip4_packet));

	return packet;
}

void ip4_sendmsg(struct socket *sock, struct sk_buff *skb)
{
	struct inet_sock *isk = inet_sk(sock->sk);
	// NOTE: MQ 2020-05-21 We don't need to perform routing, only support one router
	skb->dev = isk->sk.dev;

	skb_push(skb, sizeof(struct ethernet_packet));
	skb->mac.eh = (struct ethernet_packet *)skb->data;
	uint8_t *dest_mac = lookup_mac_addr_for_ethernet(skb->dev, isk->dsin.sin_addr);
	ethernet_build_header(skb->mac.eh, ETH_P_IP, skb->dev->dev_addr, dest_mac);
	ethernet_sendmsg(skb);
}

// Check ip header valid, adjust skb *data
int ip4_rcv(struct sk_buff *skb)
{
	if (htons(skb->mac.eh->type) != ETH_P_IP)
		return -EPROTO;

	struct ip4_packet *iph = (struct ip4_packet *)skb->data;
	skb->nh.iph = iph;
	skb_pull(skb, sizeof(struct ip4_packet));

	return 0;
}

// TODO: MQ 2020-06-18
// IP Fragmentation
// -> Send: split payload into fragments if it doesn't fit into single ip packet
// -> Receive: reassembly multi ip payloads if they have the same id and Flags-MF bit is enabled (except the last part)

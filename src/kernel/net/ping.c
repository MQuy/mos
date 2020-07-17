#include <kernel/memory/vmm.h>
#include <kernel/net/ip.h>
#include <kernel/net/neighbour.h>
#include <kernel/net/net.h>
#include <kernel/net/sk_buff.h>
#include <kernel/system/sysapi.h>
#include <kernel/system/time.h>
#include <kernel/utils/math.h>
#include <kernel/utils/printf.h>
#include <kernel/utils/string.h>

#include "icmp.h"

#define PING_HEADER_SIZE (sizeof(struct ip4_packet) + sizeof(struct icmp_packet))
#define PING_PAYLOAD_SIZE 30
#define PING_SIZE PING_HEADER_SIZE + PING_PAYLOAD_SIZE

static char *ttab[] = {
	"Echo Reply",
	"ICMP 1",
	"ICMP 2",
	"Dest Unreachable",
	"Source Quench",
	"Redirect",
	"ICMP 6",
	"ICMP 7",
	"Echo",
	"ICMP 9",
	"ICMP 10",
	"Time Exceeded",
	"Parameter Problem",
	"Timestamp",
	"Timestamp Reply",
	"Info Request",
	"Info Reply"};

struct sk_buff *ping_create_sk_buff(uint32_t source_ip, uint32_t dest_ip, uint8_t type, uint8_t code, uint16_t identifier, uint16_t sequence_number)
{
	struct sk_buff *skb = skb_alloc(PING_HEADER_SIZE, PING_PAYLOAD_SIZE);

	// payload
	skb_put(skb, PING_PAYLOAD_SIZE);

	// icmp
	skb_push(skb, sizeof(struct icmp_packet));
	struct icmp_packet *icmp = (struct icmp_packet *)skb->data;
	icmp->type = type;
	icmp->code = code;
	icmp->un.echo.id = htons(identifier);
	icmp->un.echo.sequence = htons(sequence_number);
	icmp->checksum = singular_checksum(icmp, skb->len);
	skb->h.icmph = icmp;

	// ip
	skb_push(skb, sizeof(struct ip4_packet));
	struct ip4_packet *ip = (struct ip4_packet *)skb->data;
	ip4_build_header(ip, skb->len, IP4_PROTOCAL_ICMP, source_ip, dest_ip, rand());
	skb->nh.iph = ip;

	return skb;
}

int ping_parse_from_ip_packet(struct ip4_packet *ip, struct icmp_packet **icmp)
{
	int ret = ip4_validate_header(ip, IP4_PROTOCAL_ICMP);
	if (ret < 0)
		return ret;

	struct icmp_packet *icmp_tmp = (struct icmp_packet *)((uint8_t *)ip + ip->ihl * 4);
	ret = icmp_validate_packet(icmp_tmp);
	if (ret < 0)
		return ret;

	*icmp = icmp_tmp;
	return 0;
}

// NOTE: MQ 2020-06-11
// ping below doesn't handle timeout and only support ip not hostname
// -> prove network implementation work properly
// reference: https://nsrc.org/wrc/materials/src/ping.c
void ping(uint32_t dest_ip)
{
	uint32_t sockfd = sys_socket(PF_PACKET, SOCK_DGRAM, ETH_P_IP);
	struct socket *sock = sockfd_lookup(sockfd);
	struct net_device *dev = sock->sk->dev;

	if (dev->state != NETDEV_STATE_CONNECTED)
		return;

	struct ip4_packet *received_ip = kcalloc(1, PING_SIZE);

	struct sockaddr_ll remote_sin;
	uint8_t *remote_mac_addr = lookup_mac_addr_for_ethernet(dev, dest_ip);
	memcpy(remote_sin.sll_addr, remote_mac_addr, 6);
	sock->ops->connect(sock, (struct sockaddr *)&remote_sin, sizeof(struct sockaddr_ll));

	char dest_ip_text[sizeof "255.255.255.255"];
	inet_ntop(dest_ip, dest_ip_text, sizeof(dest_ip_text));

	uint16_t identifier = rand();
	uint16_t ntransmitted = 0;
	struct icmp_packet *icmp;

	for (; ntransmitted <= 20; ntransmitted++)
	{
		struct sk_buff *skb = ping_create_sk_buff(dev->local_ip, dest_ip, ICMP_REQUEST, 0, identifier, ntransmitted);
		uint64_t sent_time = get_milliseconds(NULL);
		if (!ntransmitted)
			DEBUG &&debug_println(DEBUG_INFO, "PING %s: %d data bytes", dest_ip_text, htons(skb->nh.iph->total_length) - skb->nh.iph->ihl);
		sock->ops->sendmsg(sock, skb->nh.iph, skb->len);
		skb_free(skb);

		while (true)
		{
			memset(received_ip, 0, PING_SIZE);
			sock->ops->recvmsg(sock, received_ip, PING_SIZE);
			int ret = ping_parse_from_ip_packet(received_ip, &icmp);
			if (ret >= 0)
				break;
		}

		uint64_t received_time = get_milliseconds(NULL);
		uint32_t received_bytes = htons(received_ip->total_length) - received_ip->ihl;
		if (icmp->type != ICMP_REPLY)
			DEBUG &&debug_println(DEBUG_INFO,
								  "%d bytes from %s: icmp_type=%d (%s) icmp_code=%d",
								  received_bytes,
								  dest_ip_text,
								  icmp->type,
								  ttab[icmp->type], icmp->code);
		else
			DEBUG &&debug_println(DEBUG_INFO,
								  "%d bytes from %s: icmp_seq=%d ttl=%d time=%d",
								  received_bytes,
								  dest_ip_text,
								  htons(icmp->un.echo.sequence),
								  received_ip->time_to_live,
								  received_time - sent_time);
	}
	kfree(received_ip);
}

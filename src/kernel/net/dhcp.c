#include "dhcp.h"

#include <memory/vmm.h>
#include <net/arp.h>
#include <net/ethernet.h>
#include <net/ip.h>
#include <net/neighbour.h>
#include <net/net.h>
#include <net/sk_buff.h>
#include <net/udp.h>
#include <shared/errno.h>
#include <shared/if_ether.h>
#include <system/sysapi.h>
#include <utils/math.h>
#include <utils/printf.h>
#include <utils/string.h>

#define MAX_PACKET_LEN 576
#define MAX_UDP_HEADER (sizeof(struct ethernet_packet) + sizeof(struct ip4_packet) + sizeof(struct udp_packet))
#define DHCP_SIZE(option_len) (MAX_UDP_HEADER + sizeof(struct dhcp_packet) + (option_len))
#define DEFAULT_MTU 1500

void dhcp_build_header(struct dhcp_packet *dhp, uint8_t op, uint16_t size)
{
	dhp->op = op;
	dhp->htype = DHCP_ETHERNET;
	dhp->hlen = 6;
	dhp->hops = 0;
	dhp->magic = htonl(DHCP_MAGIC);
}

struct sk_buff *dhcp_create_skbuff(uint8_t op, uint32_t source_ip, uint32_t dest_ip, uint32_t xip, uint32_t server_ip, uint8_t *options, uint32_t options_len)
{
	struct net_device *dev = get_current_net_device();
	uint16_t dhcp_packet_size = sizeof(struct dhcp_packet) + options_len;
	struct sk_buff *skb = skb_alloc(MAX_UDP_HEADER, dhcp_packet_size);

	skb_put(skb, dhcp_packet_size);
	struct dhcp_packet *dhp = (struct dhcp_packet *)skb->data;
	dhcp_build_header(dhp, op, dhcp_packet_size);
	dhp->xip = htonl(xip);
	dhp->secs = 0;
	dhp->flags = 0;
	dhp->ciaddr = 0;
	dhp->siaddr = htonl(server_ip);
	dhp->giaddr = 0;
	memcpy(dhp->chaddr, dev->dev_addr, 6);
	memcpy(dhp->options, options, options_len);

	skb_push(skb, sizeof(struct udp_packet));
	skb->h.udph = (struct udp_packet *)skb->data;
	uint16_t udp_packet_size = sizeof(struct udp_packet) + dhcp_packet_size;
	udp_build_header(skb->h.udph, udp_packet_size, source_ip, 68, dest_ip, 67);

	skb_push(skb, sizeof(struct ip4_packet));
	skb->nh.iph = (struct ip4_packet *)skb->data;
	uint16_t ip4_packet_size = sizeof(struct ip4_packet) + udp_packet_size;
	ip4_build_header(skb->nh.iph, ip4_packet_size, IP4_PROTOCAL_UDP, source_ip, dest_ip, rand());

	skb_push(skb, sizeof(struct ethernet_packet));
	skb->mac.eh = (struct ethernet_packet *)skb->data;
	ethernet_build_header(skb->mac.eh, ETH_P_IP, dev->dev_addr, dev->broadcast_addr);

	return skb;
}

uint8_t *dhcp_set_option_value(uint8_t *options, uint8_t code, uint8_t len, void *value)
{
	options[0] = code;
	options[1] = len;
	memcpy(options + 2, value, len);

	return options + 2 + len;
}

uint32_t dhcp_get_option_value(uint8_t *options, void *value, uint8_t len)
{
	if (len == 1)
		*(uint8_t *)value = options[0];
	else if (len == 2)
		*(uint16_t *)value = options[0] + (options[1] << 8);
	else if (len == 4)
		*(uint32_t *)value = options[0] + (options[1] << 8) + (options[2] << 16) + (options[3] << 24);
	else
		return -ERANGE;

	return 0;
}

void dhcp_option_adjust_len(uint32_t *len)
{
	// NOTE: MQ 2020-06-04
	// to make frame message's length is either 342 or 576 (word align)
	// option's length is either
	if (*len <= 60)
		*len = 60;
	else
		*len = 294;
}

void dhcp_create_discovery_options(uint8_t **options, uint32_t *len)
{
	struct net_device *dev = get_current_net_device();
	*options = kcalloc(1, MAX_PACKET_LEN);
	uint8_t *iter_opt = *options;

	// DHCP Message Type
	iter_opt = dhcp_set_option_value(iter_opt, 53, 1, &(uint8_t[]){1});
	// DHCP Parameter Request List
	iter_opt = dhcp_set_option_value(iter_opt, 55, 10, &(uint8_t[]){1, 3, 6, 15, 26, 44, 46, 95, 119, 121, 252});
	// DHCP Maximum Message Size
	iter_opt = dhcp_set_option_value(iter_opt, 57, 2, &(uint8_t[]){0x05, 0xdc});
	// DHCP Client Identifier
	iter_opt = dhcp_set_option_value(iter_opt, 61, 7, &(uint8_t[]){1, dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2], dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]});
	// DHCP IP Lease Time
	iter_opt = dhcp_set_option_value(iter_opt, 51, 4, &(uint8_t[]){0, 0x76, 0xa7, 0});
	// DHCP Host name
	iter_opt = dhcp_set_option_value(iter_opt, 12, 3, &(uint8_t[]){'m', 'O', 'S'});
	// DHCP Options end
	iter_opt[0] = 0xFF;

	*len = iter_opt + 1 - *options;
	dhcp_option_adjust_len(len);
}

void dhcp_create_request_options(uint8_t **options, uint32_t *len, uint32_t requested_ip, uint32_t server_ip)
{
	struct net_device *dev = get_current_net_device();
	*options = kcalloc(1, MAX_PACKET_LEN);
	uint8_t *iter_opt = *options;

	// DHCP Message Type
	iter_opt = dhcp_set_option_value(iter_opt, 53, 1, &(uint8_t[]){3});
	// DHCP Parameter Request List
	iter_opt = dhcp_set_option_value(iter_opt, 55, 10, &(uint8_t[]){1, 3, 6, 15, 26, 44, 46, 95, 119, 121, 252});
	// DHCP Maximum Message Size
	iter_opt = dhcp_set_option_value(iter_opt, 57, 2, &(uint8_t[]){0x05, 0xdc});
	// DHCP Client Identifier
	iter_opt = dhcp_set_option_value(iter_opt, 61, 7, &(uint8_t[]){1, dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2], dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]});
	// DHCP Requested IP
	iter_opt = dhcp_set_option_value(iter_opt, 50, 4, &(uint8_t[]){(requested_ip >> 24) & 0xFF, (requested_ip >> 16) & 0xFF, (requested_ip >> 8) & 0xFF, requested_ip & 0xFF});
	// DHCP Server
	iter_opt = dhcp_set_option_value(iter_opt, 54, 4, &(uint8_t[]){(server_ip >> 24) & 0xFF, (server_ip >> 16) & 0xFF, (server_ip >> 8) & 0xFF, server_ip & 0xFF});
	// DHCP Host name
	iter_opt = dhcp_set_option_value(iter_opt, 12, 3, &(uint8_t[]){'m', 'O', 'S'});
	// DHCP Options end
	iter_opt[0] = 0xFF;

	*len = iter_opt + 1 - *options;
	dhcp_option_adjust_len(len);
}

int dhcp_validate_header(struct dhcp_packet *dhcp)
{
	if (dhcp->htype != 0x01 || dhcp->hlen != 0x06 || dhcp->magic != htonl(DHCP_MAGIC))
		return -EPROTO;

	return 0;
}

int dhcp_parse_from_eh_packet(struct ethernet_packet *eh, struct dhcp_packet **dhcp)
{
	struct ip4_packet *ip = (struct ip4_packet *)((uint32_t)eh + sizeof(struct ethernet_packet));

	int32_t ret = ip4_validate_header(ip, IP4_PROTOCAL_UDP);
	if (ret < 0)
		return ret;

	struct udp_packet *udp = (struct udp_packet *)((uint32_t)ip + sizeof(struct ip4_packet));
	ret = udp_validate_header(udp, ntohl(ip->source_ip), ntohl(ip->dest_ip));
	if (ret < 0)
		return ret;

	struct dhcp_packet *dhcp_tmp = (struct dhcp_packet *)((uint32_t)udp + sizeof(struct udp_packet));
	ret = dhcp_validate_header(dhcp_tmp);
	if (ret < 0)
		return ret;

	*dhcp = dhcp_tmp;
	return 0;
}

int dhcp_offer_parse_options(uint8_t *options, uint32_t *dhcp_server_ip)
{
	uint32_t opt_dhcp_server_ip = 0;
	uint8_t opt_message_type = 0;

	for (uint32_t i = 0; options[i] != 0xFF;)
	{
		if (options[i] == 53)
			dhcp_get_option_value(&options[i + 2], &opt_message_type, 1);
		else if (options[i] == 54)
			dhcp_get_option_value(&options[i + 2], &opt_dhcp_server_ip, 4);

		i += 2 + options[i + 1];
	}

	if (opt_message_type != 0x02)
		return -EPROTO;

	*dhcp_server_ip = ntohl(opt_dhcp_server_ip);
	return 0;
}

int dhcp_ack_parse_options(uint8_t *options, uint32_t *subnet_mask, uint32_t *router_ip, uint32_t *lease_time, uint32_t *dhcp_server_ip, uint32_t *dns_server_ip, uint16_t *mtu)
{
	uint32_t opt_subnet_mask = 0, opt_router_ip = 0, opt_lease_time = 0, opt_dhcp_server_ip = 0, opt_dns_server_ip = 0;
	uint16_t opt_mtu = ntohs(DEFAULT_MTU);
	uint8_t opt_message_type = 0;

	for (uint32_t i = 0; options[i] != 0xFF;)
	{
		if (options[i] == 1)
			dhcp_get_option_value(&options[i + 2], &opt_subnet_mask, 4);
		else if (options[i] == 3)
			dhcp_get_option_value(&options[i + 2], &opt_router_ip, 4);
		else if (options[i] == 6)
			dhcp_get_option_value(&options[i + 2], &opt_dns_server_ip, 4);
		else if (options[i] == 26)
			dhcp_get_option_value(&options[i + 2], &opt_mtu, 2);
		else if (options[i] == 51)
			dhcp_get_option_value(&options[i + 2], &opt_lease_time, 4);
		else if (options[i] == 53)
			dhcp_get_option_value(&options[i + 2], &opt_message_type, 1);
		else if (options[i] == 54)
			dhcp_get_option_value(&options[i + 2], &opt_dhcp_server_ip, 4);

		i += 2 + options[i + 1];
	}

	if (opt_message_type != 5 && opt_message_type != 6)
		return -EPROTO;

	*subnet_mask = htonl(opt_subnet_mask);
	*router_ip = htonl(opt_router_ip);
	*lease_time = htonl(opt_lease_time);
	*dhcp_server_ip = htonl(opt_dhcp_server_ip);
	*dns_server_ip = htonl(opt_dns_server_ip);
	*mtu = htons(opt_mtu);
	return 0;
}

// 1. Create af packet socket with IP/UDP protocal
// 2. Create helper to construct dhcp packet
// 3. Send dhcp discovery packet
// 4. In recvmsg -> dhcp offer -> valid IP/UDP headers
// 5. Get server_ip, yiaddr and xip -> send dhcp request
// 6. In recvmsg -> dhcp ack -> valid IP/UDP headers
// 7. Get router ip, router mac address and assign them to net dev
int dhcp_setup()
{
	DEBUG &&debug_println(DEBUG_INFO, "DHCP: Initializing");

	int32_t sockfd = sys_socket(PF_PACKET, SOCK_RAW, ETH_P_ALL);
	struct socket *sock = sockfd_lookup(sockfd);
	struct net_device *dev = sock->sk->dev;
	struct sk_buff *skb;
	struct ethernet_packet *received_eh = kcalloc(1, MAX_PACKET_LEN);
	uint8_t *options;
	uint32_t router_ip, subnet_mask, lease_time, dhcp_server_ip, local_ip, dns_server_ip;
	uint16_t mtu;
	uint32_t dhcp_xip = rand();
	int32_t ret;

	if (dev->state & NETDEV_STATE_OFF)
		return -EBUSY;

	// DHCP Discovery
	DEBUG &&debug_println(DEBUG_INFO, "DHCP: Discovery");
	uint32_t dhcp_discovery_option_len;
	dhcp_create_discovery_options(&options, &dhcp_discovery_option_len);
	skb = dhcp_create_skbuff(DHCP_REQUEST, 0, 0xffffffff, dhcp_xip, 0, options, dhcp_discovery_option_len);
	kfree(options);
	sock->ops->sendmsg(sock, skb->mac.eh, DHCP_SIZE(dhcp_discovery_option_len));

	// DHCP Offer
	struct dhcp_packet *dhcp_offer;
	uint8_t attempt_discovery = 0;
	while (true)
	{
		attempt_discovery++;
		memset(received_eh, 0, MAX_PACKET_LEN);
		sock->ops->recvmsg(sock, received_eh, MAX_PACKET_LEN);

		ret = dhcp_parse_from_eh_packet(received_eh, &dhcp_offer);
		if (ret >= 0)
		{
			ret = dhcp_offer_parse_options(dhcp_offer->options, &dhcp_server_ip);
			if (ret >= 0)
				break;
		}

		// NOTE: MQ 2020-06-06
		// DHCP Server might not accept dhcp discovery in the first few times (why?)
		// after discovering, we do it again in every 5th received packet until we get dhcp offer
		// we will use the separated process (via command line) with share memory and pthread
		if (attempt_discovery % 5 == 0)
		{
			DEBUG &&debug_println(DEBUG_INFO, "DHCP: Discovery");
			uint32_t dhcp_discovery_option_len;
			dhcp_create_discovery_options(&options, &dhcp_discovery_option_len);
			skb = dhcp_create_skbuff(DHCP_REQUEST, 0, 0xffffffff, dhcp_xip, 0, options, dhcp_discovery_option_len);
			kfree(options);
			sock->ops->sendmsg(sock, skb->mac.eh, DHCP_SIZE(dhcp_discovery_option_len));
		}
	}
	DEBUG &&debug_println(DEBUG_INFO, "DHCP: Offer");

	// DHCP Request
	DEBUG &&debug_println(DEBUG_INFO, "DHCP: Request");
	uint32_t dhcp_request_option_len;
	dhcp_create_request_options(&options, &dhcp_request_option_len, ntohl(dhcp_offer->yiaddr), ntohl(dhcp_offer->siaddr));
	skb = dhcp_create_skbuff(DHCP_REQUEST, 0, 0xffffffff, dhcp_xip, 0, options, dhcp_request_option_len);
	sock->ops->sendmsg(sock, skb->mac.eh, DHCP_SIZE(dhcp_request_option_len));
	kfree(options);

	// DHCP Ack
	struct dhcp_packet *dhcp_ack;
	while (true)
	{
		memset(received_eh, 0, MAX_PACKET_LEN);
		sock->ops->recvmsg(sock, received_eh, MAX_PACKET_LEN);

		ret = dhcp_parse_from_eh_packet(received_eh, &dhcp_ack);
		if (ret < 0)
			continue;
		ret = dhcp_ack_parse_options(dhcp_ack->options, &subnet_mask, &router_ip, &lease_time, &dhcp_server_ip, &dns_server_ip, &mtu);
		if (ret >= 0)
			break;
	}
	DEBUG &&debug_println(DEBUG_INFO, "DHCP: Ack");
	sock->ops->shutdown(sock);
	local_ip = ntohl(dhcp_ack->yiaddr);

	// ARP Announcement
	DEBUG &&debug_println(DEBUG_INFO, "DHCP: ARP Announcement");
	arp_send(dev->dev_addr, local_ip, dev->zero_addr, local_ip, ARP_REQUEST);

	dev->local_ip = local_ip;
	dev->subnet_mask = subnet_mask;
	dev->router_ip = router_ip;
	dev->lease_time = lease_time;
	dev->dhcp_server_ip = dhcp_server_ip;
	dev->dns_server_ip = dns_server_ip;
	dev->mtu = mtu;

	// ARP Probe
	DEBUG &&debug_println(DEBUG_INFO, "DHCP: ARP for router");
	uint8_t *router_mac = lookup_mac_addr_from_ip(router_ip);

	memcpy(dev->router_addr, router_mac, 6);
	dev->state = NETDEV_STATE_CONNECTED;

	kfree(received_eh);
	DEBUG &&debug_println(DEBUG_INFO, "DHCP: Done");

	return ret;
}

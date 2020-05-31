#include <include/if_ether.h>
#include <include/errno.h>
#include <kernel/memory/vmm.h>
#include <kernel/net/net.h>
#include <kernel/net/udp.h>
#include <kernel/net/arp.h>
#include <kernel/net/neighbour.h>
#include <kernel/utils/math.h>
#include "dhcp.h"

#define MAX_OPTION_LEN 60
#define MAX_DHCP_SIZE sizeof(struct ip4_packet) + sizeof(struct udp_packet) + sizeof(struct dhcp_packet) + MAX_OPTION_LEN

void dhcp_build_header(struct dhcp_packet *dhp, uint8_t op, uint16_t size)
{
  dhp->op = op;
  dhp->htype = DHCP_ETHERNET;
  dhp->hlen = 6;
  dhp->hops = 0;
  dhp->magic = htonl(DHCP_MAGIC);
}

struct sk_buff *dhcp_create_ip_packet(uint8_t op, uint32_t source_ip, uint32_t dest_ip, uint32_t xip, uint32_t server_ip, uint8_t *options, uint32_t options_len)
{
  uint16_t dhcp_packet_size = sizeof(struct dhcp_packet) + options_len;
  struct sk_buff *skb = alloc_skb(MAX_UDP_HEADER, dhcp_packet_size);

  skb_put(skb, dhcp_packet_size);
  struct dhcp_packet *dhp = skb->data;
  dhcp_build_header(dhp, op, dhcp_packet_size);
  dhp->xip = htonl(xip);
  dhp->secs = 0;
  dhp->flags = 0;
  dhp->ciaddr = 0;
  dhp->siaddr = htonl(server_ip);
  dhp->giaddr = 0;
  memcpy(dhp->chaddr, get_mac_address(), 6);
  memcpy(dhp->options, options, options_len);

  skb_pull(skb, sizeof(struct udp_packet));
  skb->h.udph = skb->data;
  uint16_t udp_packet_size = sizeof(struct udp_packet) + dhcp_packet_size;
  udp_build_header(skb->h.udph, udp_packet_size, source_ip, 68, dest_ip, 67);

  skb_pull(skb, sizeof(struct ip4_packet));
  skb->nh.iph = skb->data;
  uint16_t ip4_packet_size = sizeof(struct ip4_packet) + udp_packet_size;
  ip4_build_header(skb->nh.iph, ip4_packet_size, IP4_PROTOCAL_UDP, source_ip, dest_ip);

  return skb;
}

uint8_t *dhcp_create_discovery_options(uint32_t len)
{
  uint8_t *options = kcalloc(1, len);

  // DHCP Message Type
  options[0] = 0x35;
  options[1] = 0x01;
  options[2] = 0x01;
  // DHCP Parameter Request List
  options[3] = 0x37;
  options[4] = 0x07;
  options[5] = 0x01;
  options[6] = 0x02;
  options[7] = 0x03;
  options[8] = 0x06;
  options[9] = 0x0F;
  options[10] = 0x0c;
  options[11] = 0x1c;
  // DHCP Host name
  options[12] = 0x0C;
  options[13] = 0x03;
  options[14] = 'm';
  options[15] = 'O';
  options[16] = 'S';
  // DHCP Options end
  options[17] = 0xFF;

  return options;
}

uint8_t *dhcp_create_request_options(uint32_t len, uint32_t requested_ip, uint32_t server_ip)
{
  uint8_t *options = kcalloc(1, len);

  // DHCP Message Type
  options[0] = 0x35;
  options[1] = 0x01;
  options[2] = 0x03;
  // DHCP Requested IP
  options[3] = 50;
  options[4] = 0x04;
  options[5] = (requested_ip >> 24) & 0xFF;
  options[6] = (requested_ip >> 16) & 0xFF;
  options[7] = (requested_ip >> 8) & 0xFF;
  options[8] = requested_ip & 0xFF;
  // DHCP Server
  options[9] = 54;
  options[10] = 0x04;
  options[11] = (server_ip >> 24) & 0xFF;
  options[12] = (server_ip >> 16) & 0xFF;
  options[13] = (server_ip >> 8) & 0xFF;
  options[14] = server_ip & 0xFF;
  // DHCP Host name
  options[15] = 0x0C;
  options[16] = 0x03;
  options[17] = 'm';
  options[18] = 'O';
  options[19] = 'S';
  // DHCP Parameter Request List
  options[20] = 0x37;
  options[21] = 0x07;
  options[22] = 0x01;
  options[23] = 0x02;
  options[24] = 0x03;
  options[25] = 0x06;
  options[26] = 0x0F;
  options[27] = 0x0c;
  options[28] = 0x1c;
  // DHCP Options end
  options[29] = 0xFF;
}

int32_t dhcp_validate_header(struct dhcp_packet *dhcp)
{
}

int32_t dhcp_parse_from_ip_packet(struct ip4_packet *ip, struct dhcp_packet **dhcp)
{
  int32_t ret = ip4_validate_header(ip, IP4_PROTOCAL_UDP);
  if (ret < 0)
    return ret;

  struct udp_packet *udp = (uint32_t)ip + sizeof(struct ip4_packet);
  ret = udp_validate_header(udp);
  if (ret < 0)
    return ret;

  struct dhcp_packet *dhcp_tmp = (uint32_t)udp + sizeof(struct udp_packet);
  ret = dhcp_validate_header(dhcp);
  if (ret < 0)
    return ret;

  *dhcp = dhcp_tmp;
  return 0;
}

uint32_t get_dhcp_option_value(uint8_t *options, uint32_t *value, uint8_t len)
{
  if (len == 1)
    *value = options[0];
  else if (len == 2)
    *value = options[0] + (options[1] << 8);
  else if (len == 3)
    *value = options[0] + (options[1] << 8) + (options[2] << 16);
  else if (len == 4)
    *value = options[0] + (options[1] << 8) + (options[2] << 16) + (options[3] << 24);
  else
    return -ERANGE;

  return 0;
}

int32_t parse_dhcp_offer_options(uint8_t *options, uint32_t *server_ip)
{
  uint32_t opt_server_ip;
  uint8_t opt_message_type;

  for (uint32_t i = 0; options[i] != 0xFF;)
  {
    if (options[i] == 53)
      get_dhcp_option_value(options, &opt_message_type, 1);
    else if (options[i] == 54)
      get_dhcp_option_value(options, &opt_server_ip, 4);

    i += 2 + options[i + 1];
  }

  if (opt_message_type != 0x02)
    return -EPROTO;

  *server_ip = opt_server_ip;
  return 0;
}

int32_t parse_dhcp_ack_options(uint8_t *options, uint32_t *subnet_mask, uint32_t *router_ip, uint32_t *lease_time, uint32_t *server_ip)
{
  uint32_t opt_subnet_mask, opt_router_ip, opt_lease_time, opt_server_ip;
  uint8_t opt_message_type;

  for (uint32_t i = 0; options[i] != 0xFF;)
  {
    if (options[i] == 1)
      get_dhcp_option_value(options, &opt_subnet_mask, 4);
    else if (options[i] == 3)
      get_dhcp_option_value(options, &opt_router_ip, 4);
    else if (options[i] == 51)
      get_dhcp_option_value(options, &opt_lease_time, 4);
    else if (options[i] == 53)
      get_dhcp_option_value(options, &opt_message_type, 1);
    else if (options[i] == 54)
      get_dhcp_option_value(opt_message_type, &opt_server_ip, 4);

    i += 2 + options[i + 1];
  }

  if (opt_message_type != 5 && opt_message_type != 6)
    return -EPROTO;

  *subnet_mask = opt_subnet_mask;
  *router_ip = opt_router_ip;
  *lease_time = opt_lease_time;
  *server_ip = opt_server_ip;
  return 0;
}

// 1. Create af packet socket with IP/UDP protocal
// 2. Create helper to construct dhcp packet
// 3. Send dhcp discovery packet
// 4. In recvmsg -> dhcp offer -> valid IP/UDP headers
// 5. Get server_ip, yiaddr and xip -> send dhcp request
// 6. In recvmsg -> dhcp ack -> valid IP/UDP headers
// 7. Get router ip, router mac address and assign them to net dev
int32_t dhcp_setup()
{
  uint32_t sockfd = sys_socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP));
  struct socket *sock = sockfd_lookup(sockfd);
  struct net_device *dev = sock->sk->dev;
  struct ip4_packet *rip = kcalloc(1, MAX_DHCP_SIZE);
  uint32_t router_ip, subnet_mask, lease_time, server_ip, local_ip;
  uint32_t dhcp_xip = htonl(rand());
  int32_t ret;

  if (dev->state & NETDEV_STATE_CONNECTED == 0)
    return NULL;

  // DHCP Discovery
  struct sockaddr_ll addr_remote;
  addr_remote.sll_protocol = htons(ETH_P_IP);
  addr_remote.sll_pkttype = PACKET_BROADCAST;
  sock->ops->connect(sock, &addr_remote, sizeof(struct sockaddr_ll));

  uint8_t *options = dhcp_create_discovery_options(18);
  struct ip4_packet *ip = dhcp_create_ip_packet(DHCP_REQUEST, 0, 0xffffffff, dhcp_xip, 0, options, 18);
  sock->ops->sendmsg(sock, ip, sizeof(struct ip4_packet) + sizeof(struct udp_packet) + sizeof(struct dhcp_packet) + 18);
  sock->ops->recvmsg(sock, rip, MAX_DHCP_SIZE);

  // DHCP Offer
  struct dhcp_packet *dhcp_offer;
  ret = dhcp_parse_from_ip_packet(rip, &dhcp_offer);
  if (ret < 0)
    return ret;
  ret = parse_dhcp_offer_options(dhcp_offer->options, &server_ip);
  if (ret < 0)
    return ret;

  // DHCP Request
  uint8_t *options = dhcp_create_request_options(30, ntohl(dhcp_offer->yiaddr), server_ip);
  struct ip4_packet *ip = dhcp_create_ip_packet(DHCP_REQUEST, 0, 0xffffffff, dhcp_xip, 0, options, 30);
  sock->ops->sendmsg(sock, ip, sizeof(struct ip4_packet) + sizeof(struct udp_packet) + sizeof(struct dhcp_packet) + 30);

  memset(rip, 0, MAX_DHCP_SIZE);
  sock->ops->recvmsg(sock, rip, MAX_DHCP_SIZE);

  // DHCP Ack
  struct dhcp_packet *dhcp_ack;
  ret = dhcp_parse_from_ip_packet(rip, &dhcp_ack);
  if (ret < 0)
    return ret;
  parse_dhcp_ack_options(dhcp_ack->options, &subnet_mask, &router_ip, &lease_time, &server_ip);
  local_ip = ntohl(dhcp_ack->yiaddr);

  // ARP Announcement
  arp_send(dev->dev_addr, local_ip, dev->zero_addr, local_ip, ARP_REQUEST);

  dev->local_ip = local_ip;
  dev->subnet_mask = subnet_mask;
  dev->router_ip = router_ip;
  dev->lease_time = lease_time;
  dev->state = NETDEV_STATE_CONNECTING;

  uint8_t *router_mac_address = lookup_mac_addr_from_ip(router_ip);
  if (!router_mac_address)
    return -EINVAL;

  memcpy(dev->router_addr, router_mac_address, 6);
  dev->state = NETDEV_STATE_CONNECTED;

  return ret;
}
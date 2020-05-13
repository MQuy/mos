#include <include/ctype.h>
#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/utils/math.h>
#include <kernel/net/devices/rtl8139.h>
#include <kernel/net/ethernet.h>
#include "ip.h"

#define IP4_TTL 0x80
#define CHECKSUM_MASK 0xFFFF

struct ip4_pseudo_header
{
  uint32_t source_ip;
  uint32_t dest_ip;
  uint8_t zeros;
  uint8_t protocal;
  uint16_t udp_length;
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

int32_t ip4_send_packet(uint8_t *dmac, struct ip4_packet *packet, uint16_t packet_size)
{
  return ethernet_send_packet(dmac, packet, packet_size, ETH_P_IP);
}

struct ip4_packet *ip4_create_packet(uint16_t packet_size, void *payload, uint8_t protocal, uint32_t sip, uint32_t dip)
{
  // NOTE: MQ 2020-04-19 Align ip4 packet because checksum is calculated by alignment 2
  kalign_heap(0x2);
  struct ip4_packet *packet = kcalloc(1, packet_size);
  packet->version = 4;
  packet->ihl = 5;
  packet->type_of_service = 0;
  packet->total_length = htons(packet_size);
  packet->identification = htons(0);
  packet->flags = 0;
  packet->fragment_offset = 0;
  packet->time_to_live = IP4_TTL;
  packet->protocal = protocal;
  packet->source_ip = htonl(sip);
  packet->dest_ip = htonl(dip);
  memcpy(packet->payload, payload, packet_size - sizeof(struct ip4_packet));

  packet->header_checksum = singular_checksum(packet, sizeof(struct ip4_packet));

  return packet;
}

void set_udp_checksum(struct udp_packet *udp_packet, uint16_t udp_size, uint32_t source_ip, uint32_t dest_ip)
{
  struct ip4_pseudo_header *ip4_pseudo_header = kcalloc(1, sizeof(ip4_pseudo_header));
  ip4_pseudo_header->source_ip = htonl(source_ip);
  ip4_pseudo_header->dest_ip = htonl(dest_ip);
  ip4_pseudo_header->zeros = 0;
  ip4_pseudo_header->protocal = IP4_PROTOCAL_UDP;
  ip4_pseudo_header->udp_length = htons(udp_size);

  uint32_t ip4_checksum_start = packet_checksum_start(ip4_pseudo_header, sizeof(struct ip4_pseudo_header));
  uint32_t udp_checksum_start = packet_checksum_start(udp_packet, udp_size);
  uint32_t checksum = ip4_checksum_start + udp_checksum_start;

  while (checksum > CHECKSUM_MASK)
    checksum = (checksum & CHECKSUM_MASK) + (checksum >> 16);

  udp_packet->checksum = ~checksum & CHECKSUM_MASK;
}

struct udp_packet *udp_create_packet(uint8_t *dmac, uint16_t packet_size, void *payload, uint16_t source_port, uint16_t dest_port)
{
  struct udp_packet *packet = kcalloc(1, packet_size);
  packet->source_port = htons(source_port);
  packet->dest_port = htons(dest_port);
  packet->length = htons(packet_size);
  packet->checksum = 0;
  memcpy(packet->payload, payload, packet_size - sizeof(struct udp_packet));

  return packet;
}

struct dhcp_packet *dhcp_create_packet(uint8_t op, uint16_t size)
{
  struct dhcp_packet *packet = kcalloc(1, size);
  packet->op = op;
  packet->htype = DHCP_ETHERNET;
  packet->hlen = 6;
  packet->hops = 0;
  packet->xip = htonl(rand());
  packet->magic = htonl(DHCP_MAGIC);

  return packet;
}

void dhcp_discovery()
{
  uint16_t dhcp_packet_size = sizeof(struct dhcp_packet) + 18;
  struct dhcp_packet *dhcp_packet = dhcp_create_packet(DHCP_REQUEST, dhcp_packet_size);
  dhcp_packet->secs = 0;
  dhcp_packet->flags = 0;
  dhcp_packet->ciaddr = 0;
  dhcp_packet->siaddr = 0;
  dhcp_packet->giaddr = 0;
  memcpy(dhcp_packet->chaddr, get_mac_address(), 6);

  // DHCP Message Type
  dhcp_packet->options[0] = 0x35;
  dhcp_packet->options[1] = 0x01;
  dhcp_packet->options[2] = 0x01;
  // DHCP Parameter Request List
  dhcp_packet->options[3] = 0x37;
  dhcp_packet->options[4] = 0x07;
  dhcp_packet->options[5] = 0x01;
  dhcp_packet->options[6] = 0x02;
  dhcp_packet->options[7] = 0x03;
  dhcp_packet->options[8] = 0x06;
  dhcp_packet->options[9] = 0x0F;
  dhcp_packet->options[10] = 0x0c;
  dhcp_packet->options[11] = 0x1c;
  // DHCP Host name
  dhcp_packet->options[12] = 0x0C;
  dhcp_packet->options[13] = 0x03;
  dhcp_packet->options[14] = 'm';
  dhcp_packet->options[15] = 'O';
  dhcp_packet->options[16] = 'S';
  // DHCP Options end
  dhcp_packet->options[17] = 0xFF;

  uint16_t udp_packet_size = sizeof(struct udp_packet) + dhcp_packet_size;
  struct udp_packet *udp_packet = udp_create_packet(get_mac_address(), udp_packet_size, dhcp_packet, 68, 67);
  set_udp_checksum(udp_packet, udp_packet_size, 0, 0xffffffff);

  uint16_t ip4_packet_size = sizeof(struct ip4_packet) + udp_packet_size;
  struct ip4_packet *ip4_packet = ip4_create_packet(ip4_packet_size, udp_packet, IP4_PROTOCAL_UDP, 0, 0xffffffff);

  uint8_t dmac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  ip4_send_packet(dmac, ip4_packet, ip4_packet_size);
}

void dhcp_request(uint32_t serverip, uint32_t request_ip, uint32_t xip)
{
  uint16_t dhcp_packet_size = sizeof(struct dhcp_packet) + 30;
  struct dhcp_packet *dhcp_packet = dhcp_create_packet(DHCP_REQUEST, dhcp_packet_size);
  dhcp_packet->xip = xip;
  dhcp_packet->secs = 0;
  dhcp_packet->flags = 0;
  dhcp_packet->ciaddr = 0;
  dhcp_packet->siaddr = htonl(serverip);
  dhcp_packet->giaddr = 0;
  memcpy(dhcp_packet->chaddr, get_mac_address(), 6);

  // DHCP Message Type
  dhcp_packet->options[0] = 0x35;
  dhcp_packet->options[1] = 0x01;
  dhcp_packet->options[2] = 0x03;
  // DHCP Requested IP
  dhcp_packet->options[3] = 50;
  dhcp_packet->options[4] = 0x04;
  dhcp_packet->options[5] = (request_ip >> 24) & 0xFF;
  dhcp_packet->options[6] = (request_ip >> 16) & 0xFF;
  dhcp_packet->options[7] = (request_ip >> 8) & 0xFF;
  dhcp_packet->options[8] = request_ip & 0xFF;
  // DHCP Server
  dhcp_packet->options[9] = 54;
  dhcp_packet->options[10] = 0x04;
  dhcp_packet->options[11] = (serverip >> 24) & 0xFF;
  dhcp_packet->options[12] = (serverip >> 16) & 0xFF;
  dhcp_packet->options[13] = (serverip >> 8) & 0xFF;
  dhcp_packet->options[14] = serverip & 0xFF;
  // DHCP Host name
  dhcp_packet->options[15] = 0x0C;
  dhcp_packet->options[16] = 0x03;
  dhcp_packet->options[17] = 'm';
  dhcp_packet->options[18] = 'O';
  dhcp_packet->options[19] = 'S';
  // DHCP Parameter Request List
  dhcp_packet->options[20] = 0x37;
  dhcp_packet->options[21] = 0x07;
  dhcp_packet->options[22] = 0x01;
  dhcp_packet->options[23] = 0x02;
  dhcp_packet->options[24] = 0x03;
  dhcp_packet->options[25] = 0x06;
  dhcp_packet->options[26] = 0x0F;
  dhcp_packet->options[27] = 0x0c;
  dhcp_packet->options[28] = 0x1c;
  // DHCP Options end
  dhcp_packet->options[29] = 0xFF;

  uint16_t udp_packet_size = sizeof(struct udp_packet) + dhcp_packet_size;
  struct udp_packet *udp_packet = udp_create_packet(get_mac_address(), udp_packet_size, dhcp_packet, 68, 67);
  set_udp_checksum(udp_packet, udp_packet_size, 0, 0xffffffff);

  uint16_t ip4_packet_size = sizeof(struct ip4_packet) + udp_packet_size;
  struct ip4_packet *ip4_packet = ip4_create_packet(ip4_packet_size, udp_packet, IP4_PROTOCAL_UDP, 0, 0xffffffff);

  uint8_t dmac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  ip4_send_packet(dmac, ip4_packet, ip4_packet_size);
}

void dhcp_release(uint8_t *dmac, uint32_t cip, uint32_t server_ip)
{
  uint16_t dhcp_packet_size = sizeof(struct dhcp_packet) + 10;
  struct dhcp_packet *dhcp_packet = dhcp_create_packet(DHCP_REQUEST, dhcp_packet_size);
  dhcp_packet->secs = 0;
  dhcp_packet->flags = 0;
  dhcp_packet->ciaddr = htonl(cip);
  dhcp_packet->siaddr = 0;
  dhcp_packet->giaddr = 0;
  memcpy(dhcp_packet->chaddr, get_mac_address(), 6);
  // DHCP Message Type
  dhcp_packet->options[0] = 0x35;
  dhcp_packet->options[1] = 0x01;
  dhcp_packet->options[2] = 0x07;
  // DHCP Server Identifier
  dhcp_packet->options[3] = 0x36;
  dhcp_packet->options[4] = 0x04;
  dhcp_packet->options[5] = (server_ip >> 24) & 0xFF;
  dhcp_packet->options[6] = (server_ip >> 16) & 0xFF;
  dhcp_packet->options[7] = (server_ip >> 8) & 0xFF;
  dhcp_packet->options[8] = server_ip & 0xFF;
  // DHCP Options end
  dhcp_packet->options[9] = 0xFF;

  uint16_t udp_packet_size = sizeof(struct udp_packet) + dhcp_packet_size;
  struct udp_packet *udp_packet = udp_create_packet(get_mac_address(), udp_packet_size, dhcp_packet, 68, 67);

  uint16_t ip4_packet_size = sizeof(struct ip4_packet) + udp_packet_size;
  struct ip4_packet *ip4_packet = ip4_create_packet(ip4_packet_size, udp_packet, IP4_PROTOCAL_UDP, cip, server_ip);

  ip4_send_packet(dmac, ip4_packet, ip4_packet_size);
}

void icmp_reply(uint8_t *dmac, uint32_t cip, uint32_t server_ip, uint16_t identifier, uint16_t seq_number)
{
  uint16_t icmp_packet_size = sizeof(struct icmp_packet) + 20;
  struct icmp_packet *icmp_packet = kcalloc(1, icmp_packet_size);
  icmp_packet->type = ICMP_REPLY;
  icmp_packet->code = 0;
  icmp_packet->rest_of_header = htonl(identifier << 16 | seq_number);

  icmp_packet->checksum = singular_checksum(icmp_packet, icmp_packet_size);

  uint16_t ip4_packet_size = sizeof(struct ip4_packet) + icmp_packet_size;
  struct ip4_packet *ip4_packet = ip4_create_packet(ip4_packet_size, icmp_packet, IP4_PROTOCAL_ICMP, cip, server_ip);

  ip4_send_packet(dmac, ip4_packet, ip4_packet_size);
}

void icmp_echo(uint8_t *dmac, uint32_t cip, uint32_t server_ip)
{
  uint16_t icmp_packet_size = sizeof(struct icmp_packet) + 20;
  struct icmp_packet *icmp_packet = kcalloc(1, icmp_packet_size);
  icmp_packet->type = ICMP_ECHO;
  icmp_packet->code = 0;
  icmp_packet->rest_of_header = htonl(rand());

  icmp_packet->checksum = singular_checksum(icmp_packet, icmp_packet_size);

  uint16_t ip4_packet_size = sizeof(struct ip4_packet) + icmp_packet_size;
  struct ip4_packet *ip4_packet = ip4_create_packet(ip4_packet_size, icmp_packet, IP4_PROTOCAL_ICMP, cip, server_ip);

  ip4_send_packet(dmac, ip4_packet, ip4_packet_size);
}
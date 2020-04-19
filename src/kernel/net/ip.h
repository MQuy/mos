#ifndef NET_IP_HEADER
#define NET_IP_HEADER

#include <stdint.h>

#define IP4_PROTOCAL_UDP 17
#define DHCP_REQUEST 1
#define DHCP_REPLY 2
#define DHCP_ETHERNET 1
#define DHCP_MAGIC 0x63825363

struct __attribute__((packed)) ip4_packet
{
  uint8_t ihl : 4;
  uint8_t version : 4;
  uint8_t type_of_service;
  uint16_t total_length;
  uint16_t identification;
  uint16_t fragment_offset : 13;
  uint8_t flags : 3;
  uint8_t time_to_live;
  uint8_t protocal;
  uint16_t header_checksum;
  uint32_t source_ip;
  uint32_t dest_ip;
  uint8_t payload[];
};

struct __attribute__((packed)) udp_packet
{
  uint16_t source_port;
  uint16_t dest_port;
  uint16_t length;
  uint16_t checksum;
  uint8_t payload[];
};

struct __attribute__((packed)) dhcp_packet
{
  uint8_t op;
  uint8_t htype;
  uint8_t hlen;
  uint8_t hops;
  uint32_t xip;
  uint16_t secs;
  uint16_t flags;
  uint32_t ciaddr;
  uint32_t yiaddr;
  uint32_t siaddr;
  uint32_t giaddr;
  uint8_t chaddr[16];
  uint8_t sname[64];
  uint8_t file[128];
  uint32_t magic;
  uint8_t options[];
};

void dhcp_discovery();
void dhcp_request(uint32_t serverip, uint32_t request_ip);
void dhcp_release(uint8_t *dmac, uint32_t cip, uint32_t server_ip);

#endif
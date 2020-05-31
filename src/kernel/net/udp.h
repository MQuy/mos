#ifndef NET_UDP_H
#define NET_UDP_H

#include <stdint.h>

struct __attribute__((packed)) udp_packet
{
  uint16_t source_port;
  uint16_t dest_port;
  uint16_t length;
  uint16_t checksum;
  uint8_t payload[];
};

struct ip4_pseudo_header
{
  uint32_t source_ip;
  uint32_t dest_ip;
  uint8_t zeros;
  uint8_t protocal;
  uint16_t udp_length;
};

void udp_build_header(struct udp_packet *udp, uint16_t msg_len, uint32_t source_ip, uint16_t source_port, uint32_t dest_ip, uint16_t dest_port);
int32_t udp_validate_header(struct udp_packet *udp);

#endif
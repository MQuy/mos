#include <include/ctype.h>
#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/net/rtl8139.h>
#include "ethernet.h"

int32_t ethernet_send_packet(uint8_t *dmac, uint8_t *payload, uint32_t size, uint16_t protocal)
{
  uint32_t eh_size = sizeof(struct ethernet_header) + size;
  struct ethernet_header *eh = kmalloc(eh_size);

  eh->type = htons(protocal);
  memcpy(eh->dmac, dmac, sizeof(eh->dmac));
  memcpy(eh->smac, get_mac_address(), sizeof(eh->smac));
  memcpy(eh->payload, payload, eh_size);

  rtl8139_send_packet(eh, eh_size);

  return 0;
}
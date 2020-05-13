#include <kernel/memory/vmm.h>
#include <kernel/net/route.h>
#include "net.h"

int inet_create(struct socket *sock, int protocol)
{
}

void push_rx_queue(uint8_t *data, uint32_t size)
{
  struct sk_buff *buf = kcalloc(1, sizeof(struct sk_buff));
}

void net_init()
{
  route_init();
}

#include "raw.h"

#include <include/errno.h>
#include <include/if_ether.h>
#include <memory/vmm.h>
#include <net/ethernet.h>
#include <net/ip.h>
#include <net/neighbour.h>
#include <net/net.h>
#include <net/sk_buff.h>
#include <proc/task.h>
#include <utils/debug.h>
#include <utils/math.h>
#include <utils/string.h>

#define RAW_HEADER_SIZE (sizeof(struct ethernet_packet) + sizeof(struct ip4_packet))

int raw_bind(struct socket *sock, struct sockaddr *myaddr, int sockaddr_len)
{
	struct inet_sock *isk = inet_sk(sock->sk);
	memcpy(&isk->ssin, myaddr, sockaddr_len);
	return 0;
}

int raw_connect(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len)
{
	struct inet_sock *isk = inet_sk(sock->sk);
	memcpy(&isk->dsin, vaddr, sockaddr_len);

	sock->state = SS_CONNECTED;
	return 0;
}

int raw_sendmsg(struct socket *sock, void *msg, size_t msg_len)
{
	if (sock->state == SS_DISCONNECTED)
		return -ESHUTDOWN;

	struct inet_sock *isk = inet_sk(sock->sk);
	struct sk_buff *skb = skb_alloc(RAW_HEADER_SIZE, msg_len);
	skb->sk = sock->sk;
	skb->dev = isk->sk.dev;

	// increase tail -> copy msg into data-tail space
	skb_put(skb, msg_len);
	memcpy(skb->data, msg, msg_len);

	// decase data -> copy ip4 header into newdata-olddata
	skb_push(skb, sizeof(struct ip4_packet));
	skb->nh.iph = (struct ip4_packet *)skb->data;
	ip4_build_header(skb->nh.iph, skb->len, sock->protocol, isk->ssin.sin_addr, isk->dsin.sin_addr, 0);

	ip4_sendmsg(sock, skb);
	return 0;
}

int raw_recvmsg(struct socket *sock, void *msg, size_t msg_len)
{
	if (sock->state == SS_DISCONNECTED)
		return -ESHUTDOWN;

	struct sock *sk = sock->sk;
	struct sk_buff *skb = NULL;

	while (!skb)
	{
		skb = list_first_entry_or_null(&sk->rx_queue, struct sk_buff, sibling);
		if (!skb)
		{
			update_thread(sk->owner_thread, THREAD_WAITING);
			schedule();
		}
	}

	list_del(&skb->sibling);

	skb->mac.eh = (struct ethernet_packet *)skb->data;

	skb_pull(skb, sizeof(struct ethernet_packet));
	skb->nh.iph = (struct ip4_packet *)skb->data;

	uint32_t payload_len = min(msg_len, ntohs(skb->nh.iph->total_length) - sizeof(struct ip4_packet));
	memcpy(msg, (char *)skb->nh.iph + sizeof(struct ip4_packet), payload_len);

	skb_free(skb);
	return payload_len;
	;
}

int raw_handler(struct socket *sock, struct sk_buff *skb)
{
	if (sock->state == SS_DISCONNECTED)
		return -ESHUTDOWN;

	struct inet_sock *isk = inet_sk(sock->sk);
	struct net_device *dev = sock->sk->dev;
	struct ethernet_packet *eh = (struct ethernet_packet *)skb->data;

	if (htons(eh->type) != ETH_P_IP)
		return -EPROTO;

	skb->mac.eh = eh;
	skb_pull(skb, sizeof(struct ethernet_packet));

	struct ip4_packet *iph = (struct ip4_packet *)skb->data;
	if (htonl(iph->source_ip) == isk->dsin.sin_addr && htonl(iph->dest_ip) == dev->local_ip && iph->protocal == sock->protocol)
	{
		skb->nh.iph = iph;

		list_add_tail(&skb->sibling, &sock->sk->rx_queue);
		update_thread(sock->sk->owner_thread, THREAD_READY);
	}
	return 0;
}

struct proto_ops raw_proto_ops = {
	.family = PF_INET,
	.obj_size = sizeof(struct inet_sock),
	.bind = raw_bind,
	.ioctl = inet_ioctl,
	.connect = raw_connect,
	.sendmsg = raw_sendmsg,
	.recvmsg = raw_recvmsg,
	.shutdown = socket_shutdown,
	.handler = raw_handler,
};

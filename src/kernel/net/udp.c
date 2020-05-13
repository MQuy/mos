#include <kernel/net/net.h>
#include "udp.h"

int udp_bind(struct socket *sock,
             struct sockaddr *myaddr,
             int sockaddr_len)
{
}

int udp_connect(struct socket *sock,
                struct sockaddr *vaddr,
                int sockaddr_len, int flags)
{
}

int udp_listen(struct socket *sock, int len)
{
}

int udp_shutdown(struct socket *sock, int flags)
{
}

int udp_sendmsg(char *msg, struct socket *sock, size_t total_len)
{
}

int udp_recvmsg(char *msg, struct socket *sock, size_t total_len, int flags)
{
}

struct proto_ops inet_dgram_ops = {
    .family = PF_INET,
    .bind = udp_bind,
    .connect = udp_connect,
    .listen = udp_listen,
    .shutdown = udp_shutdown,
    .sendmsg = udp_sendmsg,
    .recvmsg = udp_recvmsg,
};
#include <kernel/net/net.h>
#include "raw.h"

int raw_bind(struct socket *sock, struct sockaddr *myaddr, int sockaddr_len)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  memcpy(&isk->ssin, myaddr, sockaddr_len);
}

int raw_connect(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  memcpy(&isk->dsin, vaddr, sockaddr_len);
}

int raw_sendmsg(struct socket *sock, char *msg, size_t msg_len)
{
}

int raw_recvmsg(struct socket *sock, char *msg, size_t msg_len)
{
}

struct proto_ops raw_proto_ops = {
    .family = PF_INET,
    .bind = raw_bind,
    .connect = raw_connect,
    .sendmsg = raw_sendmsg,
    .recvmsg = raw_recvmsg,
}
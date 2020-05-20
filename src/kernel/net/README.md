### Physical

Signal in Ethernet card (organ pair for transmit, green pair for receive) is a baud, the common way to classify bits is

- bit 0: transition from positive to negative voltage
- bit 1: transition fom negative to postive voltage

### DataLink

There are two types datalink types

- Point to point (practically in large infrastructures like internet server provider, between countries/cities)
- Multipoint/Broadcast

Byte (8 bits) is used as the computer primiary unit, but how to group 8 bits? they are just the line of bits. To solve that we define frame, there are many frame formats

- HDLC (High Level DataLink Control)
  --- Flag (frame delimiter) - Address - Control - Protocal - Payload - FCS - Flag ---
- Ethernet frame
  --- IFG - Preamble - SFD - Payload - FCS ---
  Frame's size is from 64->1500 bytes (jump frame might be longer)
  IFG (inter-frame gap) the silient between two frames synchronize between sender/receiver clock
  The reason we divide into frames is prevent any errors happned like receiver to misread, out of sync ...

### Network

OSI model is based on the encapsulation -> above layers are built based on lower layers. In this case, network protocal is located in ethernet frame's payload

Demontrate transimition computer A 192.168.9.2 (255.255.255.000) -> computer B 172.217.22.174 (255.255.000.000, google.com)

- A uses ARP (broadcast with router ip) to store router information (ip, mac address ...)
- A checks B's ip (using subnet mask) -> B is not in A's LAN
- A prepares ethernet frame with
  destination mac = router's mac (B's mac if B in the same network)
  destination ip = B's ip (172.217.22.174)
  payload = ip packet
- local router uses NAT to translate private IP to public IP
- each routers might have use different datalink protocals like PPP (Point-to-Point Protocal) but they both understand IP (Internet Protocal)
  assume that there are a bunch of routers between A and B, how router know which line to send? Each router builds its own routing table, prefix ip mask (xxx.xxx.xxx/24) - ethernet line. When checking ip packet, router matches against ip masks and forwards a packet the correspond line

  ![packet transition](https://i.imgur.com/xLbpqvd.png)
  Reference https://www.youtube.com/playlist?list=PLowKtXNTBypH19whXTVoG3oKSuOcw_XeW

**How NAT works**
The router maintains a NAT forwarding table. When receiving a packet from locals, router modifies source's (private ip, port) -> router's (global ip, random port) and keep track into the NAT forwarding table

![NAT](https://i.imgur.com/TzZGCJu.gif)

## Linux Network Packet

### Receiving a packet

**Datalink**

- nic receives the packet -> transfer into a `rx_ring` through DMA
- nic raises an interupt
- the interrupt handler
  - allocates `sk_buff` -> fill packet into `sk_buff`
  - invoke `netif_rx` which appends `sk_buff` into a queue and schedule `NET_RX_SOFTIRQ` softirq
- NET_RX_SOFTIRQ softirq is implemented by `net_rx_action`
  - extract the first packet (`sk_buff`) from the queue
  - go through list of protocal handlers (for example IPv4, ARP)
  - run receive function (for example `arp_rcv`, `ip_rcv`) for each matched protocal

**IPv4**

- check length and checksum -> discard if corrupted or truncated
- invoke `ip_route_input` to initialize the `sk_buff`'s destination cache (inet sockets can use the same `dst_entry *`)
  -> whether packet must be forwarded to another host (`ip_forward`) or passed to to the transport layer (`ip_local_delivery`)
- Look at protocal field inside ip packet
  -> call transport-level handlers (tcp_v4_rcv, udp_rcv, icmp_rcv ...)

**UDP**

- `udp_v4_lookup` to find the inet socket (`sock`) -> discard if there is no connected inet socket
- `udp_queue_rcv_skb` to append `sk_buff` into inet socket's queue (`receive_queue`)
- call `data_ready` which wakes up any sleeping process in inet socket 's `sleep` queue

so when a process reads from bsd socket owning inet socket, the following steps happen

- `read` -> `sock_recvmsg` -> `inet_recvmsg` which invokes either `tcp_recvmsg` or `udp_recvmsg`
- `udp_recvmsg`
  - invokes `skb_recv_datagram` to extract first `sk_buff` from `receive_queue`
  - copy payload of udp datagram

### Sending a packet

**Application**

```c
int sockfd; /* socket descriptor */
struct sockaddr_in addr_local, addr_remote; /* IPv4 address descriptors */
const char *mesg[] = "Hello, how are you?";

sockfd = socket(PF_INET, SOCK_DGRAM, 0);

addr_local.sin_family = AF_INET;
addr_local.sin_port = htons(50000);
addr_local.sin_addr.s_addr = htonl(0xc0a050f0); /* 192.160.80.240 */
bind(sockfd, (struct sockaddr *) & addr_local, sizeof(struct sockaddr_in));

addr_remote.sin_family = AF_INET;
addr_remote.sin_port = htons(49152);
inet_pton(AF_INET, "192.160.80.110", &addr_remote.sin_addr);
connect(sockfd, (struct sockaddr *) &addr_remote, sizeof(struct sockaddr_in));

write(sockfd, mesg, strlen(mesg)+1);
```

Credited by (Understanding the linux)[https://learning.oreilly.com/library/view/understanding-the-linux]

- `socket`
  - allocates a file descriptor, file object (inode in `sockfs`) and bsd socket (fd -> inode -> socket)
  - initialize network architecture, comminication model and protocal (inet socket, `sock` via `inet_create`)
- `bind`
  - check ip address/port number is valid
  - set local ip address/port number to inet socket
- `connect`
  - if doesn't have a port, `inet_autobind` is called to assign an unused port
  - search in `rtable` -> if not exist, invokes `ip_route_output_slow` to lookup an entry in the `FIB`

**UDP**

- `write` -> `sock_sendmsg` -> `inet_sendmsg` which invokes either `tcp_sendmsg` or `udp_sendmsg`
- `udp_sendmsg`
  - allocates `udpfakehdr` which contains UDP header
  - determinates `rtable` (routing table from `sock`'s `dst_cache`)
  - invokes `ip_build_xmit` with `sock`, UDP header and `rtable`

**IPv4**

- `ip_build_xmit`
  - invokes `sock_alloc_send_skb` to allocate a new socket buffer `sk_buff` and fill in (payload, ip header)
  - invokes `output` (implemented by `ip_output` to fill hardward address) method of`dst_entry`-> `dev_queue_xmit`

**Datalink**

- `dev_queue_xmit` takes care of queueing `sk_buff` for later transmission (usually FIFO)
  - enqueue `sk_buff` via `Qdisc`'s `enqueue` method
  - invokes `qdisc_run` to send a packet in queue
    - check network card device is idle and can transmit packets
    - if not, the queue is stopped and current `qdisc_run` is terminated -> `NET_TX_SOFTIRQ` softirq is activated. Later time, `net_tx_action` (via scheduler `ksoftirqd_CPUn`) -> `qdisc_run` (retry the transmition)
    - `hard_start_xmit` transfer `sk_buff` to the device's memory via DMA transfer

✍️ C doesn't have object-oriented concept but Linux uses [data inheritance](https://lwn.net/Articles/446317/) for example `inet_sock`/`sock` ...

## mOS Network Flow

### Send a message

![send message](https://i.imgur.com/DYSHrIv.jpg)

### Receive a message

![receive message](https://i.imgur.com/BosmFd5.jpg)

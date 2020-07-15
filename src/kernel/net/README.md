- [OSI](#osi)
  - [Physical](#physical)
  - [DataLink](#datalink)
  - [Network](#network)
- [Linux Network Packet](#linux-network-packet)
  - [Receiving a packet](#receiving-a-packet)
  - [Sending a packet](#sending-a-packet)
- [TCP](#tcp)
  - [Connection Establistment (three way handshake)](#connection-establistment-three-way-handshake)
    - [Case 1](#case-1)
    - [Case 2 (Simultaneous)](#case-2-simultaneous)
    - [Case 3 (Old duplicated SYN before new SYN)](#case-3-old-duplicated-syn-before-new-syn)
    - [Case 4 (Half-open)](#case-4-half-open)
    - [Case 5 (Old duplicated SYN on LISTEN)](#case-5-old-duplicated-syn-on-listen)
    - [Reset generation](#reset-generation)
  - [Connection closing](#connection-closing)
    - [Case 1](#case-1-1)
    - [Case 2 (Simultanous)](#case-2-simultanous)
  - [Data transfer](#data-transfer)
    - [Case 1 (fixed window size without retransmission and congestion)](#case-1-fixed-window-size-without-retransmission-and-congestion)
    - [Case 2 (adjustable window size without retransmission and congestion)](#case-2-adjustable-window-size-without-retransmission-and-congestion)
  - [Retransmission](#retransmission)
    - [RTO Computation](#rto-computation)
    - [RTO Timer](#rto-timer)
  - [Congestion](#congestion)
- [mOS Network](#mos-network)
  - [Send a message](#send-a-message)
  - [Receive a message](#receive-a-message)
  - [TCP Implementation](#tcp-implementation)
    - [Data structure](#data-structure)
    - [Establishment](#establishment)
    - [Send data](#send-data)
    - [Receive data](#receive-data)
    - [Terminate](#terminate)
  - [Test](#test)

## OSI

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

✍ Network standard uses the most significant byte (big endian) and bit first (which is also used to demonstrate in diagram)

The diagram below is the part ip header
-> `Version + IHL`. According to the standard when **reading diagram**, `Version` is more significant than `IHL` -> in memory `IHL | Version`.

```c
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Version|  IHL  |Type of Service|          Total Length         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|         Identification        |Flags|      Fragment Offset    |
```

-> In x86, why we don't need convert least to most significant bit for each byte. The reason is ethernet nic converts the bit transmission -> upper layers don't need to worry about bit order but only byte order

![nic](https://i.imgur.com/qZ60nP7.jpg)

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

## TCP

When transmiting a segment **containing data**, a copy of segment is put on the transmission queue and timer is started. If the acknowledgment for that data is received, that copy is deleted from the queue.

### Connection Establistment (three way handshake)

#### Case 1

```c
        A                                                          B
1. CLOSED         ---                                     --- LISTEN
2. SYN-SENT       --- <SEQ=100 - CTRL=SYN>                -->
3.                <-- <SEQ=500, ACK=101 - CTRL=SYN, ACK>  --- SYN-RECEIVED
4. ESTABLISHED    --- <SEQ=101, ACK=501 - CTRL=ACK>       --> ESTABLISHED
```

#### Case 2 (Simultaneous)

```c
        A                                                          B
1. CLOSED         ---                                     --- CLOSED
2. SYN-SENT       --- <SEQ=100 - CTRL=SYN>                --> ...
2. ...            <-- <SEG=500 - CTRL=SYN>                --- SYN-SENT
3. ...            <-- <SEQ=500, ACK=101 - CTRL=ACK>       --- SYN-RECEIVED
3. SYN-RECEVIED   --- <SEQ=100, ACK=505 - CTRL=ACK>       --> ...
4. ESTABLISHED    --- <SEQ=101, ACK=501 - CTRL=ACK>       --> ESTABLISHED
```

#### Case 3 (Old duplicated SYN before new SYN)

```c
        A                                                          B
1. CLOSED         ---                                     --- LISTEN
2. SYN-SENT       --- <SEQ=100 - CTRL=SYN>                --> ...
3. >>>            --- <SEQ=90 - CTRL=SYN>                 -->
4(3).             <-- <SEQ=500, ACK=91 - CTRL=SYN, ACK>   --- SYN-RECEIVED
5(2). ...         <-- <SEQ=500, ACK=101 - CTRL=RST>       --- SYN-RECEIVED
6(4).             --- <SEQ=91 - CTRL=RST>                 --> LISTEN
7(5).             --- <SEQ=101 - CTRL=SYN>                --> (3 way handshake)
```

#### Case 4 (Half-open)

```c
        A                                                          B
1. CRASH          ---                                     --- ESTABLISHED (SEQ=300, ACK=400)
2. CLOSED         ---                                     ---
3. SYN-SENT       --- <SEQ=100 - CTRL=SYN>                -->
4.                <-- <SEQ=300, ACK=400 - CTRL=ACK>       ---
5. SYN-SENT       --- <SEQ=400 - CTRL=RST>                --> CLOSED
```

#### Case 5 (Old duplicated SYN on LISTEN)

```c
        A                                                          B
1. LISTEN         ---                                     --- LISTEN
2. >>>            --- <SEQ=100, CTRL=SYN>                 -->
3.                <-- <SEQ=500, ACK=101, CTRL=SYN, ACK>   --- SYN-RECEIVED
4.                --- <SEQ=101, CTRL=RST>                 -->
5. LISTEN         ---                                     --- LISTEN
```

#### Reset generation

- The connection doesn't exist (CLOSED), any incoming segment except RST segment -> a rest segment is sent.
- The connection is in any non-synchronized state (LISTEN, SYN-SENT, SYN-RECEIVED) and the incoming segment acknowledges something not yet sent -> a rest segment is sent.
- The connection is in a synchronized state (ESTABLISHED, FIN-WAIT-1, FIN-WAIT-2, CLOST-WAIT, CLOSING, LAST-ACT, TIME-WAIT), an unacceptable segment (ACK is wrong) -> an empty acknowledge segment contains expected ACK and SND.SEQ is sent.

✍️ ️If the incoming segment has an ACK field -> SND.SEQ = RCV.ACK, otherwise -> SND.SEQ = 0
✍️ the receiver of a rest segment validates that segment. If the receiver is in LISTEN -> ignore, in SYN-RECEIVED (been LISTEN) -> back to LISTEN, otherwise -> CLOSED

### Connection closing

#### Case 1

```c
        A                                                          B
1. ESTABLISHED    ---                                     --- ESTABLISHED
2. FIN-WAIT-1     --- <SEQ=100, ACK=500 - CTRL=FIN,ACK>   -->
3.                <-- <SEQ=500, ACK=101 - CTRL=ACK>       --- CLOSE-WAIT
4. FIN-WAIT-2     ---                                     --- ...
5. FIN-WAIT-2     <-- <SEQ=500, ACK=101 - CTRL=FIN, ACK>       --- LAST-ACK
6. TIME-WAITING   --- <SEQ=101, ACK=501 - CTRL=ACK>       -->
7. CLOSED         ---                                     --- CLOSED
```

#### Case 2 (Simultanous)

```c
        A                                                          B
1. ESTABLISHED    ---                                     --- ESTABLISHED
2. FIN-WAIT-1     --- <SEQ=100, ACK=500 - CTRL=FIN, ACK>  --> ...
2. ...            <-- <SEQ=500, ACK=100 - CTRL=FIN, ACK>  --- FIN-WAIT-1
3.                <-- <SEQ=500, ACK=101 - CTRL=ACK>       --- CLOSING
3 CLOSING         --- <SEQ=100, ACK=501 - CTRL=ACK>       --->
4. TIME-WAITING   ---                (2m)                 --- TIME-WAITING
5. CLOSED         ---                                     --- CLOSED
```

### Data transfer

#### Case 1 (fixed window size without retransmission and congestion)

```c
 Step                           Client                                                  Server
----------------------------------------------------------------------------------------------------------------------
     ---   SND.UNA | SND.NXT | SND.WND | RCV.NXT | RCV.WND   ---    SND.UNA | SND.NXT | SND.WND | RCV.NXT | RCV.WND
----------------------------------------------------------------------------------------------------------------------
  1  ---      1    |    1    |   360   |   241   |   200     ---      241   |   241   |   200   |    1    |   360
----------------------------------------------------------------------------------------------------------------------
     --- During connection establishment, the client setup   --- The same as client
     --- its pointers `SND.UNA, SND.NXT, SND.WND vs RCV.NXT  ---
     --- RCV.WND` based on the parameters exchanged during   ---
     --- setup. client SND is server RCV and vice versa.     ---
----------------------------------------------------------------------------------------------------------------------
  2  ---      1    |   141   |   360   |   241   |   200     -->      241   |   241   |   200   |    1    |   360
----------------------------------------------------------------------------------------------------------------------
     --- Client transmits the 140-byte segment               -->
     --- SEG.SEQ=1, SEQ.LEN=140                              -->
----------------------------------------------------------------------------------------------------------------------
  3  ---      1    |   141   |   360   |   241   |   200     <--      241   |   321   |   200   |   141   |   360
----------------------------------------------------------------------------------------------------------------------
     ---                                                     <-- Server receives 140-byte segment and send back
     ---                                                     <-- 80-byte segment.
     ---                                                     <-- SEG.SEQ=241, SEQ.LEN=80, SEQ.ACK=141
----------------------------------------------------------------------------------------------------------------------
 4.1 ---      1    |   141   |   360   |   241   |   200     <--      241   |   441   |   200   |   141   |   360
----------------------------------------------------------------------------------------------------------------------
     ---                                                     <-- While server segment is on its way to client, server
     ---                                                     <-- is supplied with 280 byte file to send. Server cannot
     ---                                                     <-- send all in one segment because SERVER.SND.AWND=120
     ---                                                     <-- SEG.SEQ=321, SEG.LEN=120, SEQ.ACK=141, AWND=0
     ---                                                     <-- LEFT 160 byte
----------------------------------------------------------------------------------------------------------------------
3~4.2---     141   |   141   |   360   |   321   |   200     -->      241   |   441   |   200   |   141   |   360
----------------------------------------------------------------------------------------------------------------------
     --- Client receives 80-byte segment and no data to send -->
     --- it only acknowledges server segment                 -->
     --- SEG.SEQ=141, SEQ.ACK=321                            -->
----------------------------------------------------------------------------------------------------------------------
4.1~5.1-     141   |   141   |   360   |   441   |   200     -->      241   |   441   |   200   |   141   |   360
----------------------------------------------------------------------------------------------------------------------
     --- Client receives 120-byte segment, acknowledeges     -->
     --- server segment                                      -->
     --- SEG.SEQ=141, SEQ.ACK=441                            -->
----------------------------------------------------------------------------------------------------------------------
4.2~5.2-     141   |   141   |   360   |   441   |   200     ---      321   |   441   |   200   |   141   |   360
----------------------------------------------------------------------------------------------------------------------
     ---                                                     --- Server receives CLIENT.SEG.ACK=321 (4.2),
     ---                                                     --- SERVER.SND.AWND=80 so AWND < SND.WND / 2
     ---                                                     --- for performance issue, we wait AWND >= SND.WND / 2
     ---                                                     --- LEFT 160 byte
----------------------------------------------------------------------------------------------------------------------
  6  ---     141   |   141   |   360   |   441   |   200     <--      441   |   601   |   200   |   141   |   360
----------------------------------------------------------------------------------------------------------------------
     ---                                                     <-- Server receives CLIENT.SEG.ACK=441 (5.1),
     ---                                                     <-- SERVER.SND.AWND=200. Server transmits leftover segment
     ---                                                     <-- SEG.SEQ=441, SEG.LEN=160, SEQ.ACK=141, AWND=40
     ---                                                     <-- LEFT 0 byte
----------------------------------------------------------------------------------------------------------------------
  7  ---     141   |   141   |   360   |   601   |   200     -->      441   |   601   |   200   |   141   |   360
----------------------------------------------------------------------------------------------------------------------
     --- Client receives 160-byte segment, acknowledeges     -->
     --- server segment                                      -->
     --- SEG.SEQ=141, SEQ.ACK=601                            -->
----------------------------------------------------------------------------------------------------------------------
  8  ---     141   |   141   |   360   |   601   |   200     ---      601   |   601   |   200   |   141   |   360
----------------------------------------------------------------------------------------------------------------------
     ---                                                     --- Server receives CLIENT.SEG.ACK=601 (7),
     ---                                                     --- SERVER.SND.AWND=200. Completed
```

#### Case 2 (adjustable window size without retransmission and congestion)

✍️ To simplify the demonstration, we only care one sending flow from client to server

```c
 Step                           Client                                                  Server
----------------------------------------------------------------------------------------------------------------------
     ---   SND.UNA | SND.NXT | SND.WND | RCV.NXT | RCV.WND   ---    SND.UNA | SND.NXT | SND.WND | RCV.NXT | RCV.WND
----------------------------------------------------------------------------------------------------------------------
  1  ---      1    |    1    |   360   |   241   |   200     ---      241   |   241   |   200   |    1    |   360
----------------------------------------------------------------------------------------------------------------------
     --- During connection establishment, the client setup   --- The same as client
     --- its pointers `SND.UNA, SND.NXT, SND.WND vs RCV.NXT  ---
     --- RCV.WND` based on the parameters exchanged during   ---
     --- setup. client SND is server RCV and vice versa.     ---
----------------------------------------------------------------------------------------------------------------------
  2  ---      1    |   141   |   360   |   241   |   200     -->      241   |   241   |   200   |    1    |   360
----------------------------------------------------------------------------------------------------------------------
     --- Client transmits the 140-byte segment               -->
     --- SEG.SEQ=1, SEQ.LEN=140                              -->
----------------------------------------------------------------------------------------------------------------------
  3  ---      1    |   141   |   360   |   241   |   200     <--      241   |   241   |   200   |   141   |   260
----------------------------------------------------------------------------------------------------------------------
     ---                                                     <-- Server receives 140-byte segment, only send 40 bytes
     ---                                                     <-- application, leaving 100 bytes in the buffer which
     ---                                                     <-- causes RCV.WND-100
     ---                                                     <-- SEG.WND=260, SEQ.ACK=141
----------------------------------------------------------------------------------------------------------------------
  4  ---     141   |   321   |   260   |   241   |   200     -->      241   |   241   |   200   |   141   |   260
----------------------------------------------------------------------------------------------------------------------
     --- Server changed RCV.WND causes client updates        -->
     --- CLIENT.SND.WND=SERVER.RCV.WND, also send 180 byte   -->
     --- segemnt, SEQ.AWND=80                                -->
     --- SEG.SEQ=141, SEQ.LEN=180                            -->
----------------------------------------------------------------------------------------------------------------------
  5  ---     141   |   321   |   260   |   241   |   200     <--      241   |   241   |   200   |   321   |    80
----------------------------------------------------------------------------------------------------------------------
     ---                                                     <-- Server receives 180-byte segment, server is too busy
     ---                                                     <-- so 180-byte still in the buffer, causes RCV.WND-180
     ---                                                     <-- SEG.WND=80, SEG.ACK=321
----------------------------------------------------------------------------------------------------------------------
  7  ---     141   |   321   |    80   |   241   |   200     -->      241   |   241   |   200   |   321   |    80
----------------------------------------------------------------------------------------------------------------------
     --- Client send 80-byte segment                         -->
     --- server segment                                      -->
     --- SEG.SEQ=321, SEQ.ACK=80                             -->
----------------------------------------------------------------------------------------------------------------------
  8  ---     141   |   321   |    80   |   241   |   200     <--      241   |   241   |   200   |   401   |     0
----------------------------------------------------------------------------------------------------------------------
     ---                                                     <-- Server is still busy, cannot even process leftover
     ---                                                     <-- and also 80-byte segment
     ---                                                     <-- SEG.WND=0, SEG.ACK=401
----------------------------------------------------------------------------------------------------------------------
  9  ---     141   |   401   |    0    |   241   |   200     -->      241   |   241   |   200   |   361   |     0
----------------------------------------------------------------------------------------------------------------------
     --- Because WND=0, client regularly sends the probe     -->
     --- segment to prompt server to send its current WND    -->
----------------------------------------------------------------------------------------------------------------------
  10 ---     141   |   401   |    0    |   241   |   200     <--      241   |   241   |   200   |   361   |    200
----------------------------------------------------------------------------------------------------------------------
     ---                                                     <-- After a while, leftover in the buffer is processed
     ---                                                     <-- SEG.WND=200, SEG.ACK=401
```

### Retransmission

TCP maintains the retransmission queue, a copy of each segment is put into the queue with RTT which is accumulated calculation

#### RTO Computation

Following algorithm is based on [RFC 6298](https://tools.ietf.org/html/rfc6298)

R: round-trip time measurement
R': subsequent round-trip time measurement
SRTT: smoothed round-trip time
RTTVAT: round-trip time variation
G: clock granularity (leng of TCP clock tick ~ pit)
RTO: retransmission timeout

Recommended values
G = ~100ms (1ms for Linux, ~500ms for old implementation)
alpha = 1/8
beta = 1/4
K = 4

1. Initialization
   RTO = 1 (or 3 for old implementation)
2. After the first RTT measurement (R) is made
   SRTT = R
   RTTVAR = R / 2
   RTO = SRTT + max(G, K \* RTTVAR)
3. After subsequent RTT measurements (R') is made
   RTTVAR = (1 - beta) \* RTTVAR + beta \* |SRTT - R'|
   SRTT = (1 - alpha) \* SRTT + alpha \* R'
   RTO = max(SRTT + max(G, K \* RTTVAR), 1)

RTT samples must not be made using retransmitted segments (ambiguous whether the reply was the first or later segment). An optional upper bound for RTO is at least 60s.

#### RTO Timer

1. Segment containing data is sent (including a retransmission), if timer is no running -> start timer with the current RTO
2. Outstanding data (segments) has been acknowledged, remove from queue and timer
3. If retransmission timer expires
   - Retransmit the earliest unacknowledged segemnt
   - RTO = RTO \* 2
   - Start the retransmission timer with the new RTO
   - If ACK of SYN segment expires, RTO = min(3, RTO) after 3-way handshake completes

### Congestion

cwnd: Congestion window (the amount of data the sender can transmit)
rwnd: Receiver advertised window
ssthresh: Slow start threshold (to determine to switch between slow start and congestion avoidance)
IW: initial value of cwnd
SMSS: sender maximum segment size
N: unacknowledged bytes acknowledged in the incoming ACK
FlightSize: amount of outstanding in network

Beginning of a transfer, slow start is used to slowly probe the network to determine the network capacity. SYN/ACK and ACK of SYN/ACK doesn't increase cwnd. If SYN or SYN/ACK is lost, after correcty transmitted SYN -> IW = SMSS

1. if SMSS > 2190 -> IW = 2 \* SMSS
   else SMSS > 1095 -> IW = 3 \* SMSS
   else IW = 4 \* SMSS
   ssthresh = rwnd
   cwnd = IW
2. for each good ACK received
   if cwnd < ssthresh -> cwmd += min(N, SMSS) (slow start)
   else cwmd += SMSS \* SMSS / cwnd (congestion avoidance)
3. if 3 duplicated ACK received, retransmit what appears to be a missing segment without waiting timer (fast retransmit and fast recovery)
   ssthresh = max(FlightSize / 2, 2 \* SMSS)
   cwnd = ssthresh + 3 \* SMSS
   for each additional duplicated ACK received (after third) -> cwnd += SMSS
   when good ACK received, cwnd = ssthresh
4. if segment is lost (retransmission timeout)
   ssthresh = max(FlightSize / 2, 2 \* SMSS) (skip for two times retransmission)
   cwnd = IW

## mOS Network

### Send a message

![send message](https://i.imgur.com/DYSHrIv.jpg)

### Receive a message

![receive message](https://i.imgur.com/BosmFd5.jpg)

### TCP Implementation

#### Data structure

`sk_receive_queue`, `sk_write_queue` are the doubly linked list of all pending output and input packet, respectively. `sk_send_head` maintains where in the queue the next packet is to be sent.

![tcp receive queue](https://i.imgur.com/l81wNZ8.png)

```c
struct sk_buff {
  char cb[40]; // tcp_skb_sb
  ...
}

struct tcp_sock {
  struct timer_list retransmit_timer; // retransmission timer
  struct timer_list sk_timer; // keepalive timer
}
```

timer is reset/calculated when receving an ack

#### Establishment

1. `connect`
   - initialize congestion, sender/receiver sequence and timer variables like rwnd, ROT, sequence number sender/receiver, cwnd, ssthresh, window sender, RTO, ...
   - create SYN tcp segment (sk_buff without linking to sock) -> add to `sk_write_queue`
2. run `tcp_transmit_skb`
   - starting from `sk_send_head`
   - send copied SYN segment -> sleep
   - is waked up (last pharse in step 3) -> if `sk_write_queue` is empty -> go step 5
3. `tcp_handler` (switch case branch for handle 3-way handshake) -> state == SYN and only accept SYN-ACK segment
   - traverse `sk_write_queue`
     - each skb segment covered by SEG.ACK + SEG.LEN -> free that segment
     - update `sk_send_head`
   - update timer, sender/receiver sequence variables
   - create ACK segment -> send it directly
   - resume at pausing step 2
4. SYN timer expires -> go to step 1.2 and RTO = max(RTO, 3) after 3-way handshake completes
5. if SYN segment is acknowledged -> update sock state

#### Send data

1. `sendmsg` break data into MSS segments -> add to `sk_write_queue`
2. run `tcp_transmit`
   - starting from `sk_send_head` (should only contains segments from breaking data above)
   - win = min(cwnd, rwnd) -> send copied segments fit into that window -> sleep
   - is waked up -> if `sk_write_queue` is empty -> exit
3. `tcp_handler` (switch case branch for established) -> state == ESTABLISHED and only accept ACK segment
   - duplicated ACK
     - on the first or second -> send outstanding N segments with N = min(rwnd, cwnd + 2) -> update FlightSize, new timer for each segment
     - on the third -> fast retransmit
   - otherwise -> traverse `sk_write_queue` -> each skb segment covered by SEG.ACK + SEG.LEN -> free that segment and update `sk_send_head`
   - update timer, sender/receiver sequence and congestion variables
   - resume at pausing step 2
4. `retransmit_timer` is called (timeout)
   - check segments which are sent, not acknowledged yet and `when < current_time`
   - resend and recalculate congestion and timer

#### Receive data

1. `recvmsg`
   - allocates the buffer with size
   - loop check if buffer is fulfilled or PUSH flag is marked -> step 3
2. `tcp_handler` (switch case branch for established) -> state == ESTABLISHED and only accept data segment
   - if SEG.SEQ doesn't match with expected SEQ -> create expected SEQ ACK segment -> send it directly
   - otherwise -> put the segment into `sk_receive_queue`
     - update sender/receiver sequence variables, received bytes ...
     - create ACK segment -> send it directly
   - if PUSH -> mark PUSH
   - resume at pausing step 1
3. Copy the buffer to user space -> exit

#### Terminate

1. `shutdown` -> create FIN segment -> add to `sk_write_queue`
2. run `tcp_transmit_skb`
   - starting from `sk_send_head`
   - send FIN segment -> sleep
   - is waked up (last pharse in step 3) -> exit
3. `tcp_handler` (switch case branch for fin) -> state == FIN-WAIT-1 || FIN-WAIT-2
   - if ACK segment -> state = FIN-WAIT-2
   - if FIN segment and state == FIN-WAIT-2
     - state = TIME-WATING
     - create ACK segment -> send it directly
     - resume at pausing step 2.3

✍ Ignored: IP Fragmentation, Delayed ACK, URGENT Flag

### Test

**Server scenarios**

```bash
# setup server
$ sudo vi /etc/sysctl.conf
# copy 3 lines below (remove comments)
# net.ipv4.tcp_window_scaling = 1
# net.ipv4.tcp_rmem = 4096 4096 4096
# net.ipv4.tcp_wmem = 4096 4096 4096
$ sudo sysctl -p
# linux
$ iptables -A OUTPUT -p tcp --tcp-flags RST RST -s xxx.xxx.xxx.xxx -j DROP

$ cd src/kernel/net/test && gcc case1.c -o case1
$ ./case1 40000

# setup client
$ vi main.c
# din->sin_addr = server ip in hex
# din->sin_port = 40000
```

1. server with normal handshake and termination (initiate by client, without data transfer)
2. echo server
3. client transfers data, in the middle of transmission, server returns zero window
4. client transfers data, server doesn't ack -> retransmission
5. client transfers data, server retursn duplicated ack

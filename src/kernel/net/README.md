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

Demontrate transimition computer A 192.168.9.2 (255.255.255.000) -> computer B 192.168.20.2 (255.255.255.000)

- A uses ARP (broadcast with router ip) to store router information (ip, mac address ...)
- A checks B's ip (using subnet mask) -> B is not in A's LAN
- A prepares ethernet frame with
  destination mac = router's mac
  destination ip = B's ip (192.168.20.2)
  payload = ip packet
- each routers might have use different datalink protocals like PPP (Point-to-Point Protocal) but they both understand IP (Internet Protocal)
  assume that there are a bunch of routers between A and B, how router know which line to send? Each router builds its own routing table, prefix ip mask (192.168.10/24) - ethernet line. When checking ip packet, router matches against ip masks and forwards a packet the correspond line

  ![packet transition](https://i.imgur.com/xLbpqvd.png)
  Reference https://www.youtube.com/playlist?list=PLowKtXNTBypH19whXTVoG3oKSuOcw_XeW

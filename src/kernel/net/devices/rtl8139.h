#ifndef NET_RTL8139_H
#define NET_RTL8139_H

#include <stdint.h>

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

#define RTL8139_MAC0 0
#define RTL8139_MAR0 8         /* Multicast filter. */
#define RTL8139_TxStatus0 0x10 /* Transmit status (Four 32bit registers). C mode only */
                               /* Dump Tally Conter control register(64bit). C+ mode only */
#define RTL8139_TxAddr0 0x20   /* Tx descriptors (also four 32bit). */
#define RTL8139_RxBuf 0x30
#define RTL8139_ChipCmd 0x37
#define RTL8139_RxBufPtr 0x38
#define RTL8139_RxBufAddr 0x3A
#define RTL8139_IntrMask 0x3C
#define RTL8139_IntrStatus 0x3E
#define RTL8139_TxConfig 0x40
#define RTL8139_RxConfig 0x44
#define RTL8139_Timer 0x48    /* A general-purpose counter. */
#define RTL8139_RxMissed 0x4C /* 24 bits valid, write clears. */
#define RTL8139_Cfg9346 0x50
#define RTL8139_Config0 0x51
#define RTL8139_Config1 0x52
#define RTL8139_FlashReg 0x54
#define RTL8139_MediaStatus 0x58
#define RTL8139_Config3 0x59
#define RTL8139_Config4 0x5A /* absent on RTL-8139A */
#define RTL8139_HltClk 0x5B
#define RTL8139_MultiIntr 0x5C
#define RTL8139_PCIRevisionID 0x5E
#define RTL8139_TxSummary 0x60 /* TSAD register. Transmit Status of All Descriptors*/
#define RTL8139_BasicModeCtrl 0x62
#define RTL8139_BasicModeStatus 0x64
#define RTL8139_NWayAdvert 0x66
#define RTL8139_NWayLPAR 0x68
#define RTL8139_NWayExpansion 0x6A

enum RTL8139_ChipCmdBits
{
  RTL8139_CmdReset = 0x10,
  RTL8139_CmdRxEnb = 0x08,
  RTL8139_CmdTxEnb = 0x04,
  RTL8139_RxBufEmpty = 0x01,
};

enum RTL8139_IntrStatusBits
{
  RTL8139_PCIErr = 0x8000,
  RTL8139_PCSTimeout = 0x4000,
  RTL8139_RxFIFOOver = 0x40,
  RTL8139_RxUnderrun = 0x20, /* Packet Underrun / Link Change */
  RTL8139_RxOverflow = 0x10,
  RTL8139_TxErr = 0x08,
  RTL8139_TxOK = 0x04,
  RTL8139_RxErr = 0x02,
  RTL8139_RxOK = 0x01,

  RTL8139_RxAckBits = RTL8139_RxFIFOOver | RTL8139_RxOverflow | RTL8139_RxOK,
};

enum RTL8139_rx_mode_bits
{
  RTL8139_AcceptErr = 0x20,
  RTL8139_AcceptRunt = 0x10,
  RTL8139_AcceptBroadcast = 0x08,
  RTL8139_AcceptMulticast = 0x04,
  RTL8139_AcceptMyPhys = 0x02,
  RTL8139_AcceptAllPhys = 0x01,
};

enum RTL8139_RxConfigBits
{
  /* rx fifo threshold */
  RTL8139_RxCfgFIFOShift = 13,
  RTL8139_RxCfgFIFONone = (7 << RTL8139_RxCfgFIFOShift),

  /* Max DMA burst */
  RTL8139_RxCfgDMAShift = 8,
  RTL8139_RxCfgDMAUnlimited = (7 << RTL8139_RxCfgDMAShift),

  /* rx ring buffer length */
  RTL8139_RxCfgRcv8K = 0,
  RTL8139_RxCfgRcv16K = (1 << 11),
  RTL8139_RxCfgRcv32K = (1 << 12),
  RTL8139_RxCfgRcv64K = (1 << 11) | (1 << 12),

  /* Disable packet wrap at end of Rx buffer. (not possible with 64k) */
  RTL8139_RxNoWrap = (1 << 7),
};

#define ROK 0x01
#define TOK 0x04

#define RX_BUFFER_SIZE 8096
#define RX_PADDING_BUFFER_SIZE (8096 + 16 + 1500)

#define RX_BUF_PTR_MASK 0xfffffffc
#define RX_PACKET_HEADER_MAR 0x8000
#define RX_PACKET_HEADER_PAM 0x4000
#define RX_PACKET_HEADER_BAR 0x2000
#define RX_PACKET_HEADER_ISE 0x20
#define RX_PACKET_HEADER_RUNT 0x10
#define RX_PACKET_HEADER_LONG 0x08
#define RX_PACKET_HEADER_CRC 0x04
#define RX_PACKET_HEADER_FAE 0x02
#define RX_PACKET_HEADER_ROK 0x01

struct __attribute__((packed)) rtl8139_rx_header
{
  uint16_t status;
  uint16_t size;
};

void rtl8139_init();
void rtl8139_send_packet(void *payload, uint32_t size);

#endif
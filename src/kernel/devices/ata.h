#ifndef DEVICE_ATA_H
#define DEVICE_ATA_H

#include <stdbool.h>
#include <stdint.h>

#define ATA0_IO_ADDR1 0x1F0
#define ATA0_IO_ADDR2 0x3F0
#define ATA0_IRQ 14
#define ATA1_IO_ADDR1 0x170
#define ATA1_IO_ADDR2 0x370
#define ATA1_IRQ 15
// #define ATA2_IO_ADDR1 0x1E8
// #define ATA2_IO_ADDR2 0x3E0
// #define ATA2_IRQ 11
// #define ATA3_IO_ADDR1 0x168
// #define ATA3_IO_ADDR2 0x360
// #define ATA3_IRQ 9

#define ATA_SREG_ERR 0x01
#define ATA_SREG_DF 0x20
#define ATA_SREG_DRQ 0x08
#define ATA_SREG_BSY 0x80

#define ATA_POLLING_ERR 0
#define ATA_POLLING_SUCCESS 1

#define ATA_IDENTIFY_ERR 0
#define ATA_IDENTIFY_SUCCESS 1
#define ATA_IDENTIFY_NOT_FOUND 2

typedef struct ata_device
{
  uint16_t io_base;
  uint16_t associated_io_base;
  uint8_t irq;
  bool is_master;
} ata_device;

uint8_t ata_init();
uint8_t ata_read(ata_device device, uint32_t lba, uint8_t n_sectors, uint16_t *buffer);
uint8_t ata_write(ata_device device, uint32_t lba, uint8_t n_sectors, uint16_t *buffer);

#endif
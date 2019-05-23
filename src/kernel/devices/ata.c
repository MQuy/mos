#include "../cpu/hal.h"
#include "../cpu/idt.h"
#include "../graphics/DebugDisplay.h"
#include "./ata.h"

ata_device devices[4];
uint8_t number_of_actived_devices = 0;

uint8_t ata_identify(ata_device device);
uint8_t ata_polling(ata_device device);
ata_device *ata_detect(uint16_t io_addr1, uint16_t io_addr2, uint8_t irq, bool is_master);

uint8_t ata_init()
{
  ata_device *device = ata_detect(ATA0_IO_ADDR1, ATA0_IO_ADDR2, ATA0_IRQ, true);
  ata_detect(ATA0_IO_ADDR1, ATA0_IO_ADDR2, ATA0_IRQ, false);
  ata_detect(ATA1_IO_ADDR1, ATA1_IO_ADDR2, ATA1_IRQ, true);
  ata_detect(ATA1_IO_ADDR1, ATA1_IO_ADDR2, ATA1_IRQ, false);

  if (device != 0)
  {
    // FIXME: MQ 2019-05-26 Replace pmm_alloc_block by malloc(512)
    uint16_t *buffer = pmm_alloc_block();
    ata_read(devices[0], 0, 1, buffer);
  }
}

ata_device *ata_detect(uint16_t io_addr1, uint16_t io_addr2, uint8_t irq, bool is_master)
{
  // FIXME: MQ 2019-05-26 Replace pmm_alloc_block by malloc(sizeof(ata_device))
  ata_device *device = pmm_alloc_block();
  device->io_base = io_addr1;
  device->associated_io_base = io_addr2;
  device->irq = irq;
  device->is_master = is_master;

  if (ata_identify(*device) == ATA_IDENTIFY_SUCCESS)
  {
    devices[number_of_actived_devices++] = *device;
    return device;
  }
  else
    return 0;
}

uint8_t ata_identify(ata_device device)
{
  uint8_t status;

  outportb(device.associated_io_base + 6, 0x2);

  outportb(device.io_base + 6, device.is_master ? 0xA0 : 0xB0);
  outportb(device.io_base + 2, 0);
  outportb(device.io_base + 3, 0);
  outportb(device.io_base + 4, 0);
  outportb(device.io_base + 5, 0);
  outportb(device.io_base + 7, 0xEC);

  while (true)
  {
    status = inportb(device.io_base + 7);

    if (status == 0)
      return ATA_IDENTIFY_NOT_FOUND;
    if (!(status & ATA_SREG_BSY))
      if (inportb(device.io_base + 4) || inportb(device.io_base + 5))
        return ATA_IDENTIFY_ERR;
      else
        break;
  }

  while (true)
  {
    status = inportb(device.io_base + 7);

    if (status & ATA_SREG_DRQ)
      break;
    if (status & ATA_SREG_ERR || status & ATA_SREG_DF)
      return ATA_IDENTIFY_ERR;
  }

  if (!(status & ATA_SREG_ERR))
  {
    uint16_t buffer[256];

    for (int i = 0; i < 256; ++i)
      buffer[i] = inportw(device.io_base);

    return ATA_IDENTIFY_SUCCESS;
  }
  return ATA_IDENTIFY_ERR;
}

uint8_t ata_read(ata_device device, uint32_t lba, uint8_t n_sectors, uint16_t *buffer)
{
  outportb(device.associated_io_base + 6, 0x2);

  outportb(device.io_base + 6, (device.is_master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));

  if (ata_polling(device) == ATA_POLLING_ERR)
    return;

  outportb(device.io_base + 1, 0x00);
  outportb(device.io_base + 2, n_sectors);
  outportb(device.io_base + 3, (uint8_t)lba);
  outportb(device.io_base + 4, (uint8_t)(lba >> 8));
  outportb(device.io_base + 5, (uint8_t)(lba >> 16));
  outportb(device.io_base + 7, 0x20);

  if (ata_polling(device) == ATA_POLLING_ERR)
    return;

  for (int read_sector = 0; read_sector < n_sectors; ++read_sector)
  {
    for (uint16_t i = 0; i < 256; ++i)
    {
      uint16_t data = inportw(device.io_base);
      *(buffer + i * 2 + read_sector * 256) = data;
      DebugPrintf("%x", data);
    }

    if (ata_polling(device) == ATA_POLLING_ERR)
      return;
  }
}

uint8_t ata_write(ata_device device, uint32_t lba, uint8_t n_sectors, uint16_t *buffer)
{
  outportb(device.associated_io_base + 6, 0x2);

  outportb(device.io_base + 6, (device.is_master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));

  if (ata_polling(device) == ATA_POLLING_ERR)
    return;

  outportb(device.io_base + 1, 0x00);
  outportb(device.io_base + 2, n_sectors);
  outportb(device.io_base + 3, (uint8_t)lba);
  outportb(device.io_base + 4, (uint8_t)(lba >> 8));
  outportb(device.io_base + 5, (uint8_t)(lba >> 16));
  outportb(device.io_base + 7, 0x30);

  if (ata_polling(device) == ATA_POLLING_ERR)
    return;

  for (int read_sector = 0; read_sector < n_sectors; ++read_sector)
  {

    for (uint16_t i = 0; i < 256; ++i)
    {
      outportw(device.io_base, buffer[i]);
    }

    outportw(device.io_base + 7, 0xE7);

    if (ata_polling(device) == ATA_POLLING_ERR)
      return;
  }
}

uint8_t ata_polling(ata_device device)
{
  uint8_t status;

  while (true)
  {
    status = inportb(device.io_base + 7);
    if (!(status & ATA_SREG_BSY) || (status & ATA_SREG_DRQ))
      return ATA_POLLING_SUCCESS;
    if ((status & ATA_SREG_ERR) || (status & ATA_SREG_DF))
      return ATA_POLLING_ERR;
  }
}
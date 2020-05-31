#include <include/errno.h>
#include <kernel/utils/string.h>
#include <kernel/cpu/hal.h>
#include <kernel/cpu/idt.h>
#include <kernel/memory/vmm.h>
#include "ata.h"

#define MAX_ATA_DEVICE 4

static struct ata_device devices[MAX_ATA_DEVICE];
static uint8_t number_of_actived_devices = 0;

uint8_t ata_identify(struct ata_device *device);
uint8_t atapi_identify(struct ata_device *device);
struct ata_device *ata_detect(uint16_t io_addr1, uint16_t io_addr2, uint8_t irq, bool is_master, char *dev_name);
uint8_t ata_polling(struct ata_device *device);
uint8_t ata_polling_identify(struct ata_device *device);
void ata_400ns_delays(struct ata_device *device);

volatile bool ata_irq_called;

int32_t ata_irq()
{
  ata_irq_called = true;

  return IRQ_HANDLER_CONTINUE;
}

int32_t ata_wait_irq()
{
  while (!ata_irq_called)
    ;
  ata_irq_called = false;

  return IRQ_HANDLER_CONTINUE;
}

uint8_t ata_init()
{
  register_interrupt_handler(IRQ14, ata_irq);
  register_interrupt_handler(IRQ15, ata_irq);

  ata_detect(ATA0_IO_ADDR1, ATA0_IO_ADDR2, ATA0_IRQ, true, "/dev/hda");
  ata_detect(ATA0_IO_ADDR1, ATA0_IO_ADDR2, ATA0_IRQ, false, "/dev/hdb");
  ata_detect(ATA1_IO_ADDR1, ATA1_IO_ADDR2, ATA1_IRQ, true, "/dev/hdc");
  ata_detect(ATA1_IO_ADDR1, ATA1_IO_ADDR2, ATA1_IRQ, false, "/dev/hdd");
  return 0;
}

struct ata_device *ata_detect(uint16_t io_addr1, uint16_t io_addr2, uint8_t irq, bool is_master, char *dev_name)
{
  struct ata_device *device = kcalloc(1, sizeof(struct ata_device));
  device->io_base = io_addr1;
  device->associated_io_base = io_addr2;
  device->irq = irq;
  device->is_master = is_master;

  if (ata_identify(device) == ATA_IDENTIFY_SUCCESS)
  {
    device->is_harddisk = true;
    device->dev_name = dev_name;
    devices[number_of_actived_devices++] = *device;
    return device;
  }
  else if (atapi_identify(device) == ATA_IDENTIFY_SUCCESS)
  {
    device->is_harddisk = false;
    device->dev_name = "/dev/cdrom";
    devices[number_of_actived_devices++] = *device;
    return device;
  }
  return 0;
}

uint8_t ata_identify(struct ata_device *device)
{
  outportb(device->io_base + 6, device->is_master ? 0xA0 : 0xB0);
  ata_400ns_delays(device);

  outportb(device->io_base + 2, 0);
  outportb(device->io_base + 3, 0);
  outportb(device->io_base + 4, 0);
  outportb(device->io_base + 5, 0);
  outportb(device->io_base + 7, 0xEC);

  uint8_t identify_status = ata_polling_identify(device);
  if (identify_status != ATA_IDENTIFY_SUCCESS)
    return identify_status;

  if (!(inportb(device->io_base + 7) & ATA_SREG_ERR))
  {
    uint16_t buffer[256];

    inportsw(device->io_base, buffer, 256);

    return ATA_IDENTIFY_SUCCESS;
  }
  return ATA_IDENTIFY_ERR;
}

int8_t ata_read(struct ata_device *device, uint32_t lba, uint8_t n_sectors, uint16_t *buffer)
{
  outportb(device->io_base + 6, (device->is_master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
  ata_400ns_delays(device);

  outportb(device->io_base + 1, 0x00);
  outportb(device->io_base + 2, n_sectors);
  outportb(device->io_base + 3, (uint8_t)lba);
  outportb(device->io_base + 4, (uint8_t)(lba >> 8));
  outportb(device->io_base + 5, (uint8_t)(lba >> 16));
  outportb(device->io_base + 7, 0x20);

  if (ata_polling(device) == ATA_POLLING_ERR)
    return -ENXIO;

  for (int i = 0; i < n_sectors; ++i)
  {
    inportsw(device->io_base, buffer + i * 256, 256);
    ata_400ns_delays(device);

    if (ata_polling(device) == ATA_POLLING_ERR)
      return -ENXIO;
  }
  return 0;
}

int8_t ata_write(struct ata_device *device, uint32_t lba, uint8_t n_sectors, uint16_t *buffer)
{
  outportb(device->io_base + 6, (device->is_master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
  ata_400ns_delays(device);

  outportb(device->io_base + 1, 0x00);
  outportb(device->io_base + 2, n_sectors);
  outportb(device->io_base + 3, (uint8_t)lba);
  outportb(device->io_base + 4, (uint8_t)(lba >> 8));
  outportb(device->io_base + 5, (uint8_t)(lba >> 16));
  outportb(device->io_base + 7, 0x30);

  if (ata_polling(device) == ATA_POLLING_ERR)
    return -ENXIO;

  for (int i = 0; i < n_sectors; ++i)
  {
    outportsw(device->io_base, buffer + i * 256, 256);
    outportw(device->io_base + 7, 0xE7);
    ata_400ns_delays(device);

    if (ata_polling(device) == ATA_POLLING_ERR)
      return -ENXIO;
  }
  return 0;
}

uint8_t atapi_identify(struct ata_device *device)
{
  outportb(device->io_base + 6, device->is_master ? 0xA0 : 0xB0);
  ata_400ns_delays(device);

  outportb(device->io_base + 7, 0xA1);

  uint8_t identify_status = ata_polling_identify(device);
  if (identify_status != ATA_IDENTIFY_SUCCESS)
    return identify_status;

  if (!(inportb(device->io_base + 7) & ATA_SREG_ERR))
  {
    uint16_t buffer[256];

    inportsw(device->io_base, buffer, 256);

    return ATA_IDENTIFY_SUCCESS;
  }
  return ATA_IDENTIFY_ERR;
}

int8_t atapi_read(struct ata_device *device, uint32_t lba, uint8_t n_sectors, uint16_t *buffer)
{
  uint8_t packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  outportb(device->io_base + 6, device->is_master ? 0xE0 : 0xF0);
  ata_400ns_delays(device);

  outportb(device->io_base + 1, 0);
  outportb(device->io_base + 4, (2048 & 0xff));
  outportb(device->io_base + 5, 2048 > 8);
  outportb(device->io_base + 7, 0xA0);

  ata_polling(device);

  packet[9] = n_sectors;
  packet[2] = (lba >> 0x18) & 0xFF;
  packet[3] = (lba >> 0x10) & 0xFF;
  packet[4] = (lba >> 0x08) & 0xFF;
  packet[5] = (lba >> 0x00) & 0xFF;

  outportsw(device->io_base, (uint16_t *)packet, 6);

  ata_wait_irq();
  ata_polling(device);

  for (int i = 0; i < n_sectors; ++i)
  {
    inportsw(device->io_base, buffer + 256 * i, 256);

    if (ata_polling(device) == ATA_POLLING_ERR)
      return -ENXIO;
  }
  return 0;
}

void ata_400ns_delays(struct ata_device *device)
{
  inportb(device->io_base + 7);
  inportb(device->io_base + 7);
  inportb(device->io_base + 7);
  inportb(device->io_base + 7);
}

uint8_t ata_polling(struct ata_device *device)
{
  uint8_t status;

  while (true)
  {
    status = inportb(device->io_base + 7);
    if (!(status & ATA_SREG_BSY) || (status & ATA_SREG_DRQ))
      return ATA_POLLING_SUCCESS;
    if ((status & ATA_SREG_ERR) || (status & ATA_SREG_DF))
      return ATA_POLLING_ERR;
  }
}

uint8_t ata_polling_identify(struct ata_device *device)
{
  uint8_t status;

  while (true)
  {
    status = inportb(device->io_base + 7);

    if (status == 0)
      return ATA_IDENTIFY_NOT_FOUND;
    if (!(status & ATA_SREG_BSY))
      // FIXME: MQ 2019-05-29 ATAPI never work (against https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command)
      // if (inportb(device.io_base + 4) || inportb(device.io_base + 5))
      //   return ATA_IDENTIFY_ERR;
      // else
      break;
  }

  while (true)
  {
    status = inportb(device->io_base + 7);

    if (status & ATA_SREG_DRQ)
      break;
    if (status & ATA_SREG_ERR || status & ATA_SREG_DF)
      return ATA_IDENTIFY_ERR;
  }

  return ATA_IDENTIFY_SUCCESS;
}

struct ata_device *get_ata_device(char *dev_name)
{
  for (uint8_t i = 0; i < MAX_ATA_DEVICE; ++i)
  {
    if (strcmp(devices[i].dev_name, dev_name) == 0)
      return &devices[i];
  }
  return NULL;
}

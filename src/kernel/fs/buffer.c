#include "buffer.h"
#include <kernel/utils/math.h>
#include <kernel/memory/vmm.h>
#include <kernel/devices/ata.h>

// FIXME: MQ 2019-07-16 Is it safe to assume 512 b/s, it's fairly safe for hard disk but CD ROM might use 2048 b/s
#define BYTES_PER_SECTOR 512

char *bread(char *dev_name, sector_t sector, uint32_t size)
{
  struct ata_device *device = get_ata_device(dev_name);
  char *buf = kcalloc(div_ceil(size, BYTES_PER_SECTOR) * BYTES_PER_SECTOR, sizeof(char));
  ata_read(device, sector, div_ceil(size, BYTES_PER_SECTOR), (uint16_t *)buf);
  return buf;
}

void bwrite(char *dev_name, sector_t sector, char *buf, uint32_t size)
{
  struct ata_device *device = get_ata_device(dev_name);
  ata_write(device, sector, div_ceil(size, BYTES_PER_SECTOR), (uint16_t *)buf);
}

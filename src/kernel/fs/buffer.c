#include "buffer.h"

#include <devices/ata.h>
#include <memory/vmm.h>
#include <utils/math.h>

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

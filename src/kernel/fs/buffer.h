#ifndef FS_BUFFER_H
#define FS_BUFFER_H

#include <shared/types.h>
#include <stdint.h>

char *bread(char *dev_name, sector_t block, uint32_t size);
void bwrite(char *dev_name, sector_t block, char *buf, uint32_t size);

#endif

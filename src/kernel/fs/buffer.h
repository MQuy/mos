#ifndef FS_BUFFER_H
#define FS_BUFFER_H

#include <include/types.h>
#include <stdint.h>

// FIXME: MQ 2019-07-16 Is it safe to assume 512 b/s, it's fairly safe for hard disk but CD ROM might use 2048 b/s
#define BYTES_PER_SECTOR 512

char *bread(char *dev_name, sector_t block, uint32_t size);
void bwrite(char *dev_name, sector_t block, char *buf, uint32_t size);

#endif

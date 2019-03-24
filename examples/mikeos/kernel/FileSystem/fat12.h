/******************************************************************************
   fat12.h
		-FAT12 Minidriver

   arthor\ Mike
******************************************************************************/

#ifndef FAT12_H_INCLUDED
#define FAT12_H_INCLUDED

#include <stdint.h>
#include "fsys.h"

/**
*	Directory entry
*/
typedef struct _DIRECTORY
{

	uint8_t Filename[8];
	uint8_t Ext[3];
	uint8_t Attrib;
	uint8_t Reserved;
	uint8_t TimeCreatedMs;
	uint16_t TimeCreated;
	uint16_t DateCreated;
	uint16_t DateLastAccessed;
	uint16_t FirstClusterHiBytes;
	uint16_t LastModTime;
	uint16_t LastModDate;
	uint16_t FirstCluster;
	uint32_t FileSize;

} __attribute__((packed)) DIRECTORY, *PDIRECTORY;

/**
*	Filesystem mount info
*/
typedef struct _MOUNT_INFO
{

	uint32_t numSectors;
	uint32_t fatOffset;
	uint32_t numRootEntries;
	uint32_t rootOffset;
	uint32_t rootSize;
	uint32_t fatSize;
	uint32_t fatEntrySize;

} MOUNT_INFO, *PMOUNT_INFO;

FILE fsysFatDirectory(const char *DirectoryName);
void fsysFatRead(PFILE file, unsigned char *Buffer, unsigned int Length);
FILE fsysFatOpen(const char *FileName);
void fsysFatInitialize();
void fsysFatMount();

#endif

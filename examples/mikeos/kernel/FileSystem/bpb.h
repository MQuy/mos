//****************************************************************************
//**
//**    bpb.h
//**		-Bios Paramater Block
//**
//****************************************************************************

#ifndef BPB_H
#define BPB_H

#include <stdint.h>

/**
*	bios paramater block
*/
typedef struct _BIOS_PARAMATER_BLOCK
{

	uint8_t OEMName[8];
	uint16_t BytesPerSector;
	uint8_t SectorsPerCluster;
	uint16_t ReservedSectors;
	uint8_t NumberOfFats;
	uint16_t NumDirEntries;
	uint16_t NumSectors;
	uint8_t Media;
	uint16_t SectorsPerFat;
	uint16_t SectorsPerTrack;
	uint16_t HeadsPerCyl;
	uint32_t HiddenSectors;
	uint32_t LongSectors;

} __attribute__((packed)) BIOSPARAMATERBLOCK, *PBIOSPARAMATERBLOCK;

/**
*	bios paramater block extended attributes
*/
typedef struct _BIOS_PARAMATER_BLOCK_EXT
{

	uint32_t SectorsPerFat32;
	uint16_t Flags;
	uint16_t Version;
	uint32_t RootCluster;
	uint16_t InfoCluster;
	uint16_t BackupBoot;
	uint16_t Reserved[6];

} __attribute__((packed)) BIOSPARAMATERBLOCKEXT, *PBIOSPARAMATERBLOCKEXT;

/**
*	boot sector
*/
typedef struct _BOOT_SECTOR
{

	uint8_t Ignore[3]; //first 3 bytes are ignored
	BIOSPARAMATERBLOCK Bpb;
	BIOSPARAMATERBLOCKEXT BpbExt;
	uint8_t Filler[448]; //needed to make struct 512 bytes

} __attribute__((packed)) BOOTSECTOR, *PBOOTSECTOR;

#endif

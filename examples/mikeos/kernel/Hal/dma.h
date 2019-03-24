#ifndef _DMA_H_INCLUDED
#define _DMA_H_INCLUDED
//****************************************************************************
//**
//**	dma.h
//**		8237 ISA Direct Memory Access Controller (DMAC)
//**
//****************************************************************************

//============================================================================
//    INTERFACE REQUIRED HEADERS
//============================================================================

#include <stdint.h>

//============================================================================
//    INTERFACE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================

/**
*	2 DMACs, 32 bit master & 16bit slave each having 8 channels
*/
#define DMA_MAX_CHANNELS 16
#define DMA_CHANNELS_PER_DMAC 8

/**
*	DMA0 address/count registers
*/
typedef enum DMA0_CHANNEL_IO
{

	DMA0_CHAN0_ADDR_REG = 0, //! Thats right, i/o port 0
	DMA0_CHAN0_COUNT_REG = 1,
	DMA0_CHAN1_ADDR_REG = 2,
	DMA0_CHAN1_COUNT_REG = 3,
	DMA0_CHAN2_ADDR_REG = 4,
	DMA0_CHAN2_COUNT_REG = 5,
	DMA0_CHAN3_ADDR_REG = 6,
	DMA0_CHAN3_COUNT_REG = 7,
} DMA0_CHANNEL_IO;

/**
*	Generic DMA0 registers
*/
typedef enum DMA0_IO
{

	DMA0_STATUS_REG = 0x08,
	DMA0_COMMAND_REG = 0x08,
	DMA0_REQUEST_REG = 0x09,
	DMA0_CHANMASK_REG = 0x0a,
	DMA0_MODE_REG = 0x0b,
	DMA0_CLEARBYTE_FLIPFLOP_REG = 0x0c,
	DMA0_TEMP_REG = 0x0d,
	DMA0_MASTER_CLEAR_REG = 0x0d,
	DMA0_CLEAR_MASK_REG = 0x0e,
	DMA0_MASK_REG = 0x0f
} DMA0_IO;

/**
*	DMA1 address/count registers
*/
typedef enum DMA1_CHANNEL_IO
{

	DMA1_CHAN4_ADDR_REG = 0xc0,
	DMA1_CHAN4_COUNT_REG = 0xc2,
	DMA1_CHAN5_ADDR_REG = 0xc4,
	DMA1_CHAN5_COUNT_REG = 0xc6,
	DMA1_CHAN6_ADDR_REG = 0xc8,
	DMA1_CHAN6_COUNT_REG = 0xca,
	DMA1_CHAN7_ADDR_REG = 0xcc,
	DMA1_CHAN7_COUNT_REG = 0xce,
} DMA1_CHANNEL_IO;

/**
*	DMA External Page Registers
*/
typedef enum DMA0_PAGE_REG
{

	DMA_PAGE_EXTRA0 = 0x80, //! Also diagnostics port
	DMA_PAGE_CHAN2_ADDRBYTE2 = 0x81,
	DMA_PAGE_CHAN3_ADDRBYTE2 = 0x82,
	DMA_PAGE_CHAN1_ADDRBYTE2 = 0x83,
	DMA_PAGE_EXTRA1 = 0x84,
	DMA_PAGE_EXTRA2 = 0x85,
	DMA_PAGE_EXTRA3 = 0x86,
	DMA_PAGE_CHAN6_ADDRBYTE2 = 0x87,
	DMA_PAGE_CHAN7_ADDRBYTE2 = 0x88,
	DMA_PAGE_CHAN5_ADDRBYTE2 = 0x89,
	DMA_PAGE_EXTRA4 = 0x8c,
	DMA_PAGE_EXTRA5 = 0x8d,
	DMA_PAGE_EXTRA6 = 0x8e,
	DMA_PAGE_DRAM_REFRESH = 0x8f //!no longer used in new PCs
} DMA0_PAGE_REG;

/**
*	Generic DMA1 registers
*/
typedef enum DMA1_IO
{

	DMA1_STATUS_REG = 0xd0,
	DMA1_COMMAND_REG = 0xd0,
	DMA1_REQUEST_REG = 0xd2,
	DMA1_CHANMASK_REG = 0xd4,
	DMA1_MODE_REG = 0xd6,
	DMA1_CLEARBYTE_FLIPFLOP_REG = 0xd8,
	DMA1_INTER_REG = 0xda,
	DMA1_UNMASK_ALL_REG = 0xdc,
	DMA1_MASK_REG = 0xde
} DMA1_IO;

/**
*	DMA mode bit mask (Non MASK Values can be ORed together)
*/

typedef enum DMA_MODE_REG_MASK
{

	DMA_MODE_MASK_SEL = 3,

	DMA_MODE_MASK_TRA = 0xc,
	DMA_MODE_SELF_TEST = 0,
	DMA_MODE_READ_TRANSFER = 4,
	DMA_MODE_WRITE_TRANSFER = 8,

	DMA_MODE_MASK_AUTO = 0x10,
	DMA_MODE_MASK_IDEC = 0x20,

	DMA_MODE_MASK = 0xc0,
	DMA_MODE_TRANSFER_ON_DEMAND = 0,
	DMA_MODE_TRANSFER_SINGLE = 0x40,
	DMA_MODE_TRANSFER_BLOCK = 0x80,
	DMA_MODE_TRANSFER_CASCADE = 0xC0
} DMA_MODE_REG_MASK;

typedef enum DMA_CMD_REG_MASK
{

	DMA_CMD_MASK_MEMTOMEM = 1,
	DMA_CMD_MASK_CHAN0ADDRHOLD = 2,
	DMA_CMD_MASK_ENABLE = 4,
	DMA_CMD_MASK_TIMING = 8,
	DMA_CMD_MASK_PRIORITY = 0x10,
	DMA_CMD_MASK_WRITESEL = 0x20,
	DMA_CMD_MASK_DREQ = 0x40,
	DMA_CMD_MASK_DACK = 0x80
} DMA_CMD_REG_MASK;

//============================================================================
//    INTERFACE CLASS PROTOTYPES / EXTERNAL CLASS REFERENCES
//============================================================================
//============================================================================
//    INTERFACE STRUCTURES / UTILITY CLASSES
//============================================================================
//============================================================================
//    INTERFACE DATA DECLARATIONS
//============================================================================
//============================================================================
//    INTERFACE FUNCTION PROTOTYPES
//============================================================================

//! sets the mode of a channel
void dma_set_mode(uint8_t channel, uint8_t mode);

//! prepares for generic channel read
void dma_set_read(uint8_t channel);

//! prepares for generic channel write
void dma_set_write(uint8_t channel);

//! sets the address of a channel
void dma_set_address(uint8_t channel, uint8_t low, uint8_t high);

//! sets the counter of a channel
void dma_set_count(uint8_t channel, uint8_t low, uint8_t high);

//! masks a channel
void dma_mask_channel(uint8_t channel);

//! unmasks a channel
void dma_unmask_channel(uint8_t channel);

//! resets a flipflop
void dma_reset_flipflop(int dma);

//! reset the dma to defaults
void dma_reset(int dma);

//! sets an external page register
void dma_set_external_page_register(uint8_t reg, uint8_t val);

//! unmasks all registers
void dma_unmask_all(int dma);

//============================================================================
//    INTERFACE OBJECT CLASS DEFINITIONS
//============================================================================
//============================================================================
//    INTERFACE TRAILING HEADERS
//============================================================================
//****************************************************************************
//**
//**    END [dma.h]
//**
//****************************************************************************
#endif

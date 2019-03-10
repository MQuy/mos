//**
//**	main.cpp
//**		entry point for kernel
//**
//****************************************************************************

#include "bootinfo.h"
#include "mmngr_phys.h"
#include "mmngr_virtual.h"
#include "DebugDisplay.h"

//! format of a memory region
typedef struct
{

  uint32_t startLo;
  uint32_t startHi;
  uint32_t sizeLo;
  uint32_t sizeHi;
  uint32_t type;
  uint32_t acpi_3_0;
} memory_region;

//! different memory regions (in memory_region.type)
char *strMemoryTypes[] = {

    {"Available"},      //memory_region.type==0
    {"Reserved"},       //memory_region.type==1
    {"ACPI Reclaim"},   //memory_region.type==2
    {"ACPI NVS Memory"} //memory_region.type==3
};

int start(multiboot_info *bootinfo)
{

  //! get kernel size passed from boot loader
  // HACK: I don't know why it has to be plus one to correct kernel's size
  uint32_t kernelSize = bootinfo->m_kernel_size + 1;
  // or get it directly from edx register
  // __asm__ __volatile__(""
  //                      : "=ed"(kernelSize));

  //! clear and init display
  DebugClrScr(0x13);
  DebugGotoXY(0, 0);
  DebugSetColor(0x17);
  DebugPrintf("M.O.S. Kernel is starting up...\n");

  pmmngr_init(bootinfo->m_memorySize, 0xC0000000 + kernelSize * 512);

  DebugGotoXY(0, 3);
  DebugSetColor(0x19);
  DebugPrintf("pmm initialized with %i KB\n", bootinfo->m_memorySize);

  memory_region *region = (memory_region *)0x1000;

  for (int i = 0; i < 10; ++i)
  {

    if (region[i].type > 4)
      break;

    if (i > 0 && region[i].startLo == 0)
      break;

    DebugPrintf("region %i: start: 0x%x%x length (bytes): 0x%x%x type: %i (%s)\n", i,
                region[i].startHi, region[i].startLo,
                region[i].sizeHi, region[i].sizeLo,
                region[i].type, strMemoryTypes[region[i].type - 1]);

    pmmngr_init_region(region[i].startLo, region[i].sizeLo);
  }
  pmmngr_deinit_region(0x100000, kernelSize * 512);
  // NOTE: Why we need to deinit region 0x0->0x10000? if not it doesn't work
  pmmngr_deinit_region(0x0, 0x10000);
  DebugPrintf("pmm regions initialized: %i allocation blocks; block size: %i bytes",
              pmmngr_get_block_count(), pmmngr_get_block_size());

  //! initialize our vmm
  vmmngr_initialize();

  if (pmmngr_is_paging())
  {
    DebugPrintf("\nPDT's address: 0x%x", pmmngr_get_PDBR());
  }
  else
  {
    DebugPrintf("\nPaging is failed");
  }
  return 0;
}
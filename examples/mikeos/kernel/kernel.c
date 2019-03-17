//**
//**	main.cpp
//**		entry point for kernel
//**
//****************************************************************************

#include "bootinfo.h"
#include "exception.h"
#include "Hal/Hal.h"
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

    "Available",      //memory_region.type==0
    "Reserved",       //memory_region.type==1
    "ACPI Reclaim",   //memory_region.type==2
    "ACPI NVS Memory" //memory_region.type==3
};

int start(multiboot_info *bootinfo)
{
  __asm__ __volatile__(
      "cli\n"
      "movw $0x10, %ax \n"
      "movw %ax, %ds \n"
      "movw %ax, %es \n"
      "movw %ax, %fs \n"
      "movw %ax, %gs \n");

  //! get kernel size passed from boot loader
  // HACK: I don't know why it has to be plus one to correct kernel's size
  uint32_t kernelSize = bootinfo->m_kernel_size + 1;

  //! clear and init display
  DebugClrScr(0x13);
  DebugGotoXY(0, 0);
  DebugSetColor(0x17);
  DebugPrintf("M.O.S. Kernel is starting up...\n");

  hal_initialize();

  //! enable all interrupts
  enable();

  // //! install our exception handlers
  setvect(0, (I86_IRQ_HANDLER)divide_by_zero_fault);
  setvect(1, (I86_IRQ_HANDLER)single_step_trap);
  setvect(2, (I86_IRQ_HANDLER)nmi_trap);
  setvect(3, (I86_IRQ_HANDLER)breakpoint_trap);
  setvect(4, (I86_IRQ_HANDLER)overflow_trap);
  setvect(5, (I86_IRQ_HANDLER)bounds_check_fault);
  setvect(6, (I86_IRQ_HANDLER)invalid_opcode_fault);
  setvect(7, (I86_IRQ_HANDLER)no_device_fault);
  setvect(8, (I86_IRQ_HANDLER)double_fault_abort);
  setvect(10, (I86_IRQ_HANDLER)invalid_tss_fault);
  setvect(11, (I86_IRQ_HANDLER)no_segment_fault);
  setvect(12, (I86_IRQ_HANDLER)stack_fault);
  setvect(13, (I86_IRQ_HANDLER)general_protection_fault);
  setvect(14, (I86_IRQ_HANDLER)page_fault);
  setvect(16, (I86_IRQ_HANDLER)fpu_fault);
  setvect(17, (I86_IRQ_HANDLER)alignment_check_fault);
  setvect(18, (I86_IRQ_HANDLER)machine_check_abort);
  setvect(19, (I86_IRQ_HANDLER)simd_fpu_fault);

  pmmngr_init(bootinfo->m_memorySize, 0xC0000000 + kernelSize * 512);

  DebugGotoXY(0, 3);
  DebugSetColor(0x19);
  DebugPrintf("pmm initialized with %i KB\n", bootinfo->m_memorySize);

  memory_region *region = (memory_region *)bootinfo->m_mmap_addr;

  for (int i = 0; i < bootinfo->m_mmap_length; ++i)
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

  sleep(1000);

  DebugPrintf("\n\nShutdown complete. Halting system");

  __asm__ __volatile__(
      "cli \n"
      "hlt \n");
  for (;;)
    ;

  return 0;
}

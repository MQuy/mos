//**
//**		entry point for kernel
//**
//****************************************************************************

#include "bootinfo.h"
#include "exception.h"
#include "Include/string.h"
#include "Hal/Hal.h"
#include "Keyboard/kybrd.h"
#include "mmngr_phys.h"
#include "mmngr_virtual.h"
#include "DebugDisplay.h"

//! format of a memory region
typedef struct memory_region
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
  //! get kernel size passed from boot loader
  // HACK: I don't know why it has to be plus more than 9 to correct kernel's size
  uint32_t kernelSize = bootinfo->m_kernel_size + 9;

  //! clear and init display
  DebugClrScr(0x13);
  DebugGotoXY(0, 0);
  DebugSetColor(0x17);
  DebugPrintf("M.O.S. Kernel is starting up...\n");

  hal_initialize();

  //! install our exception handlers
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

  // !initialize our vmm
  vmmngr_initialize();

  kkybrd_install(33);

  run();

  for (;;)
    ;

  return 0;
}

//! sleeps a little bit. This uses the HALs get_tick_count() which in turn uses the PIT
void sleep(int ms)
{
  int currentTick = get_tick_count();
  int endTick = ms + currentTick;
  while (endTick > get_tick_count())
    ;
}

//! wait for key stroke
KEYCODE getch()
{

  KEYCODE key = KEY_UNKNOWN;

  //! wait for a keypress
  while (key == KEY_UNKNOWN)
    key = kkybrd_get_last_key();

  //! discard last keypress (we handled it) and return
  kkybrd_discard_last_key();
  return key;
}

//! command prompt
void cmd()
{

  DebugPrintf("\nCommand> ");
}

//! gets next command
void get_cmd(char *buf, int n)
{

  cmd();

  KEYCODE key = KEY_UNKNOWN;
  bool BufChar;

  //! get command string
  int i = 0;
  while (i < n)
  {

    //! buffer the next char
    BufChar = true;

    //! grab next char
    key = getch();

    //! end of command if enter is pressed
    if (key == KEY_RETURN)
      break;

    //! backspace
    if (key == KEY_BACKSPACE)
    {

      //! dont buffer this char
      BufChar = false;

      if (i > 0)
      {

        //! go back one char
        unsigned y, x;
        DebugGetXY(&x, &y);
        if (x > 0)
          DebugGotoXY(--x, y);
        else
        {
          //! x is already 0, so go back one line
          y--;
          x = DebugGetHorz();
        }

        //! erase the character from display
        DebugPutc(' ');
        DebugGotoXY(x, y);

        //! go back one char in cmd buf
        i--;
      }
    }

    //! only add the char if it is to be buffered
    if (BufChar)
    {

      //! convert key to an ascii char and put it in buffer
      char c = kkybrd_key_to_ascii(key);
      if (c != 0)
      { //insure its an ascii char

        DebugPutc(c);
        buf[i++] = c;
      }
    }

    //! wait for next key. You may need to adjust this to suite your needs
    sleep(10);
  }

  //! null terminate the string
  buf[i] = '\0';
}

//! our simple command parser
bool run_cmd(char *cmd_buf)
{

  //! exit command
  if (strcmp(cmd_buf, "exit") == 0)
  {
    return true;
  }

  //! clear screen
  else if (strcmp(cmd_buf, "cls") == 0)
  {
    DebugClrScr(0x17);
  }

  //! help
  else if (strcmp(cmd_buf, "help") == 0)
  {

    DebugPuts("\nOS Development Series Keyboard Programming Demo");
    DebugPuts("\nwith a basic Command Line Interface (CLI)\n\n");
    DebugPuts("Supported commands:\n");
    DebugPuts(" - exit: quits and halts the system\n");
    DebugPuts(" - cls: clears the display\n");
    DebugPuts(" - help: displays this message\n");
  }

  //! invalid command
  else
  {
    DebugPrintf("\nUnkown command");
  }

  return false;
}

void run()
{

  //! command buffer
  char cmd_buf[100];

  while (1)
  {

    //! get command
    get_cmd(cmd_buf, 98);

    //! run command
    if (run_cmd(cmd_buf) == true)
      break;
  }
}
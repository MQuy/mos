//**
//**		entry point for kernel
//**
//****************************************************************************

#include "bootinfo.h"
#include "Hal/Hal.h"
#include "Hal/tss.h"
#include "Keyboard/kybrd.h"
#include "Include/string.h"
#include "Include/stdio.h"
#include "FileSystem/flpydsk.h"
#include "FileSystem/fat12.h"

#include "DebugDisplay.h"
#include "sysapi.h"
#include "task.h"
#include "exception.h"
#include "mmngr_phys.h"
#include "mmngr_virtual.h"

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

extern void enter_usermode();

void get_cmd(char *buf, int n);
void cmd_read();
void cmd_read_sect();
void cmd_thread();
void cmd_user();
void run();
bool run_cmd(char *cmd_buf);

int kernel_main(multiboot_info *bootinfo)
{
  __asm__ __volatile__("cli  \n"
                       "mov $0x10, %ax \n"
                       "mov %ax, %ds \n"
                       "mov %ax, %es \n"
                       "mov %ax, %fs \n"
                       "mov %ax, %gs \n");
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

  pmmngr_init((size_t)bootinfo->m_memorySize, 0xC0000000 + kernelSize * 512);

  memory_region *region = (memory_region *)bootinfo->m_mmap_addr;

  for (int i = 0; i < bootinfo->m_mmap_length; ++i)
  {

    if (region[i].type > 4)
      break;

    if (i > 0 && region[i].startLo == 0)
      break;

    pmmngr_init_region(region[i].startLo, region[i].sizeLo);
  }
  pmmngr_deinit_region(0x100000, kernelSize * 512);
  // NOTE: Why we need to deinit region 0x0->0x10000? if not it doesn't work
  pmmngr_deinit_region(0x0, 0x10000);

  // !initialize our vmm
  vmmngr_initialize();

  kkybrd_install(33);

  //! set drive 0 as current drive
  flpydsk_set_working_drive(0);

  //! install floppy disk to IR 38, uses IRQ 6
  flpydsk_install(38);

  //! initialize FAT12 filesystem
  fsysFatInitialize();

  //! initialize system calls
  syscall_init();

  //! initialize TSS
  install_tss(5, 0x10, 0);

  DebugGotoXY(0, 0);
  DebugPuts("OSDev Series VFS Programming Demo");
  DebugPuts("\nType \"exit\" to quit, \"help\" for a list of commands\n");
  DebugPuts("+-------------------------------------------------------+\n");

  run();

  DebugPrintf("\nExit command recieved; demo halted");

  for (;;)
    ;

  return 0;
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

//! gets next command
void get_cmd(char *buf, int n)
{

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

//! read command
void cmd_read()
{

  //! get pathname
  char path[32];
  DebugPrintf("\n\rex: \"file.txt\", \"a:\\file.txt\", \"a:\\folder\\file.txt\"\n\rFilename> ");
  get_cmd(path, 32);

  //! open file
  FILE file = volOpenFile(path);

  //! test for invalid file
  if (file.flags == FS_INVALID)
  {

    DebugPrintf("\n\rUnable to open file");
    return;
  }

  //! cant display directories
  if ((file.flags & FS_DIRECTORY) == FS_DIRECTORY)
  {

    DebugPrintf("\n\rUnable to display contents of directory.");
    return;
  }

  //! top line
  DebugPrintf("\n\n\r-------[%s]-------\n\r", file.name);

  //! display file contents
  while (file.eof != 1)
  {

    //! read cluster
    unsigned char buf[512];
    volReadFile(&file, buf, 512);

    //! display file
    for (int i = 0; i < 512; i++)
      DebugPutc(buf[i]);

    //! wait for input to continue if not EOF
    if (file.eof != 1)
    {
      DebugPrintf("\n\r------[Press a key to continue]------");
      getch();
      DebugPrintf("\r"); //clear last line
    }
  }

  //! done :)
  DebugPrintf("\n\n\r--------[EOF]--------");
}

//! read sector command
void cmd_read_sect()
{

  uint32_t sectornum = 0;
  char sectornumbuf[4];
  uint8_t *sector = 0;

  DebugPrintf("\n\rPlease type in the sector number [0 is default] >");
  get_cmd(sectornumbuf, 3);
  sectornum = atoi(sectornumbuf);

  DebugPrintf("\n\rSector %i contents:\n\n\r", sectornum);

  //! read sector from disk
  sector = flpydsk_read_sector(sectornum);

  //! display sector
  if (sector != 0)
  {

    int i = 0;
    for (int c = 0; c < 4; c++)
    {

      for (int j = 0; j < 128; j++)
        DebugPrintf("0x%x ", sector[i + j]);
      i += 128;

      DebugPrintf("\n\rPress any key to continue\n\r");
      getch();
    }
  }
  else
    DebugPrintf("\n\r*** Error reading sector from disk");

  DebugPrintf("\n\rDone.");
}

// BUG: MQ 2019-03-31 Something is wrong with stackframe causes page fault
// To make it work, disable `pmmngr_init_region/pmmngr_deinit_region`, seem that memory mapping (Memory.inc) is not correct
void cmd_user()
{

  int esp;
  __asm__ __volatile__("mov %%esp, %0"
                       : "=r"(esp));
  tss_set_stack(0x10, esp);

  enter_usermode();
  syscall_printf("\nIn user mode");
}

//! our simple command parser
bool run_cmd(char *cmd_buf)
{
  //! thread
  if (strcmp(cmd_buf, "thread") == 0)
  {
    cmd_thread();
  }

  //! user mode
  else if (strcmp(cmd_buf, "user") == 0)
  {
    cmd_user();
  }

  //! exit command
  else if (strcmp(cmd_buf, "exit") == 0)
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

    DebugPuts("\nOS Development Series VFS Programming Demo");
    DebugPuts("Supported commands:\n");
    DebugPuts(" - exit: quits and halts the system\n");
    DebugPuts(" - cls: clears the display\n");
    DebugPuts(" - help: displays this message\n");
    DebugPuts(" - read: reads a file\n");
    DebugPuts(" - user: goes to user mode\n");
    DebugPuts(" - thread: runs multithreading\n");
    DebugPuts(" - reset: Resets and recalibrates floppy for reading\n");
  }

  //! read sector
  else if (strcmp(cmd_buf, "read") == 0)
  {
    cmd_read();
  }

  //! read sector
  else if (strcmp(cmd_buf, "readsect") == 0)
  {
    cmd_read_sect();
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

    //! display prompt
    DebugPrintf("\nCommand> ");

    //! get command
    get_cmd(cmd_buf, 98);

    //! run command
    if (run_cmd(cmd_buf) == true)
      break;
  }
}

/* sleep for some time. */
void sleep(int ms)
{

  if (ms > 1)
    thread_sleep(ms);
}

/*** BGA Support ****************************************/

/* We'll be needing this. */
void outportw(unsigned short portid, unsigned short value)
{
  __asm__("out %%ax, %%dx"
          :
          : "a"(value), "d"(portid));
}

/* Definitions for BGA. Reference Graphics 1. */
#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF
#define VBE_DISPI_INDEX_XRES 0x1
#define VBE_DISPI_INDEX_YRES 0x2
#define VBE_DISPI_INDEX_BPP 0x3
#define VBE_DISPI_INDEX_ENABLE 0x4
#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_LFB_ENABLED 0x40

/* writes to BGA port. */
void VbeBochsWrite(uint16_t index, uint16_t value)
{
  outportw(VBE_DISPI_IOPORT_INDEX, index);
  outportw(VBE_DISPI_IOPORT_DATA, value);
}

/* sets video mode. */
void VbeBochsSetMode(uint16_t xres, uint16_t yres, uint16_t bpp)
{
  VbeBochsWrite(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
  VbeBochsWrite(VBE_DISPI_INDEX_XRES, xres);
  VbeBochsWrite(VBE_DISPI_INDEX_YRES, yres);
  VbeBochsWrite(VBE_DISPI_INDEX_BPP, bpp);
  VbeBochsWrite(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
}

/* video mode info. */
#define WIDTH 800
#define HEIGHT 600
#define BPP 32
#define BYTES_PER_PIXEL 4

/* BGA LFB is at LFB_PHYSICAL. */
#define LFB_PHYSICAL 0xE0000000
#define LFB_VIRTUAL 0x300000
#define PAGE_SIZE 0x1000

/* map LFB into current process address space. */
void *VbeBochsMapLFB()
{
  int pfcount = WIDTH * HEIGHT * BYTES_PER_PIXEL / PAGE_SIZE;
  for (int c = 0; c <= pfcount; c++)
    vmmngr_mapPhysicalAddress(vmmngr_get_directory(), LFB_VIRTUAL + c * PAGE_SIZE, LFB_PHYSICAL + c * PAGE_SIZE, 7);
  return (void *)LFB_VIRTUAL;
}

/* clear screen to white. */
void fillScreen32()
{
  uint32_t *lfb = (uint32_t *)LFB_VIRTUAL;
  for (uint32_t c = 0; c < WIDTH * HEIGHT; c++)
    lfb[c] = 0xffffffff;
}

/* render rectangle in 32 bpp modes. */
void rect32(int x, int y, int w, int h, int col)
{
  uint32_t *lfb = (uint32_t *)LFB_VIRTUAL;
  for (uint32_t k = 0; k < h; k++)
    for (uint32_t j = 0; j < w; j++)
      lfb[(j + x) + (k + y) * WIDTH] = col;
}

/* thread cycles through colors of red. */
void kthread_1()
{
  int col = 0;
  bool dir = true;
  while (1)
  {
    rect32(200, 250, 100, 100, col << 16);
    if (dir)
    {
      if (col++ == 0xfe)
        dir = false;
    }
    else if (col-- == 1)
      dir = true;
  }
}

/* thread cycles through colors of green. */
void kthread_2()
{
  int col = 0;
  bool dir = true;
  while (1)
  {
    rect32(350, 250, 100, 100, col << 8);
    if (dir)
    {
      if (col++ == 0xfe)
        dir = false;
    }
    else if (col-- == 1)
      dir = true;
  }
}

/* thread cycles through colors of blue. */
void kthread_3()
{
  int col = 0;
  bool dir = true;
  while (1)
  {
    rect32(500, 250, 100, 100, col);
    if (dir)
    {
      if (col++ == 0xfe)
        dir = false;
    }
    else if (col-- == 1)
      dir = true;
  }
}

void cmd_thread()
{
  /* set video mode and map framebuffer. */
  VbeBochsSetMode(WIDTH, HEIGHT, BPP);
  VbeBochsMapLFB();
  fillScreen32();

  /* init scheduler. */
  scheduler_initialize();

  // /* create kernel threads. */
  queue_insert(thread_create(kthread_1, (uint32_t)create_kernel_stack(), true));
  queue_insert(thread_create(kthread_2, (uint32_t)create_kernel_stack(), true));
  queue_insert(thread_create(kthread_3, (uint32_t)create_kernel_stack(), true));

  // /* execute idle thread. */
  execute_idle();
}

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "include/multiboot.h"
#include "graphics/DebugDisplay.h"
#include "cpu/hal.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/pit.h"
#include "devices/kybrd.h"
#include "memory/pmm.h"
#include "memory/vmm.h"

bool run_cmd(char *cmd_buf);
void run();
KEYCODE getch();
void get_cmd(char *buf, int n);

int kernel_main(uint32_t boot_magic, multiboot_info_t *boot_info)
{

  if (boot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
  {
    DebugPrintf("\nBootloader MAGIC is invalid... Halting");
    return -1;
  }

  DebugClrScr(0x13);
  DebugGotoXY(0, 0);
  DebugSetColor(0x17);

  gdt_init();
  idt_init();
  pit_init();

  kkybrd_install();

  enable_interrupts();

  pmm_init(boot_info);
  vmm_init();

  run();

  for (;;)
    ;

  return 0;
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
  }

  //! null terminate the string
  buf[i] = '\0';
}

//! our simple command parser
bool run_cmd(char *cmd_buf)
{
  //! thread
  if (strcmp(cmd_buf, "thread") == 0)
  {
    DebugPrintf("\nthread");
  }

  //! user mode
  else if (strcmp(cmd_buf, "user") == 0)
  {
    DebugPrintf("\nuser");
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
    DebugPrintf("\nread");
  }

  //! read sector
  else if (strcmp(cmd_buf, "readsect") == 0)
  {
    DebugPrintf("\nreadsect");
  }

  //! invalid command
  else
  {
    DebugPrintf("\nUnkown command");
  }

  return false;
}
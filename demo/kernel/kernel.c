#include "DebugDisplay.h"

void start()
{
  int i = 0x12;

  DebugClrScr(0x18);

  DebugGotoXY(4, 4);
  DebugSetColor(0x17);
  DebugPrintf("+-----------------------------------------+\n");
  DebugPrintf("|    MOS 32 Bit C++ Kernel Executing!     |\n");
  DebugPrintf("+-----------------------------------------+\n\n");
  DebugSetColor(0x12);

  DebugSetColor(0x12);
  DebugPrintf("\ni as integer ........................");
  DebugPrintf("\ni in hex ............................");

  DebugGotoXY(25, 8);
  DebugSetColor(0x1F);
  DebugPrintf("\n[%i]", i);
  DebugPrintf("\n[0x%x]", i);

  DebugGotoXY(4, 16);
  DebugSetColor(0x1F);
  DebugPrintf("\n\nI am preparing to load... Hold on, please... :)");
}

// #include "drivers/screen.h"
// #include "util.h"

// void start()
// {
//   clear_screen();

//   /* Fill up the screen */
//   int i = 0;
//   for (i = 0; i < 24; i++)
//   {
//     char str[255] = "text1th2";
//     kprint_at(str, 0, i);
//   }

//   kprint_at("This text forces the kernel to scroll. Row 0 will disappear. ", 60, 24);
//   kprint("And with this text, the kernel will scroll again, and row 1 will disappear too!");
// }
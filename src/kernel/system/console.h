#ifndef SYSTEM_CONSOLE_H
#define SYSTEM_CONSOLE_H

#include <stdarg.h>
#include <stdint.h>
#include <kernel/multiboot2.h>

struct framebuffer
{
  uint32_t addr;
  uint32_t pitch;
  uint32_t width;
  uint32_t height;
  uint8_t bpp;
};

void console_init(struct multiboot_tag_framebuffer *);
void console_setup();
struct framebuffer *get_framebuffer();
int printf(const char *fmt, ...);

#endif

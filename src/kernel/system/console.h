#ifndef SYSTEM_CONSOLE_H
#define SYSTEM_CONSOLE_H

#include <stdarg.h>
#include <stdint.h>
#include <kernel/include/multiboot2.h>

void console_init(struct multiboot_tag_framebuffer *);
int printf(const char *fmt, ...);

#endif
#ifndef FONTS_PSF_H
#define FONTS_PSH_H

#include <stdint.h>
#include <stddef.h>

#define PSF_FONT_MAGIC 0x864ab572
#define PSF_HAS_UNICODE_TABLE 0x01

struct psf_t
{
  uint32_t magic;
  uint32_t version;
  uint32_t headersize;
  uint32_t flags;
  uint32_t numglyph;
  uint32_t bytesperglyph;
  uint32_t height;
  uint32_t width;
};

void psf_init(char *buff, size_t size);
void psf_putchar(
    uint32_t c,
    uint32_t cx, uint32_t cy,
    uint32_t fg, uint32_t bg, char *fb, uint32_t scanline);
void psf_puts(
    const char *s,
    uint32_t cx, uint32_t cy,
    uint32_t fg, uint32_t bg, char *fb, uint32_t scanline);

#endif

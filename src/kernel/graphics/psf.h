#ifndef GRAPHICS_PSF_H
#define GRAHPICS_PSH_H

#include <stdint.h>
#include <stddef.h>

#define PSF_FONT_MAGIC 0x864ab572
#define PSF_HAS_UNICODE_TABLE 0x01

typedef struct
{
  uint32_t magic;
  uint32_t version;
  uint32_t headersize;
  uint32_t flags;
  uint32_t numglyph;
  uint32_t bytesperglyph;
  uint32_t height;
  uint32_t width;
} psf_t;

void psf_init(char *psf_start, size_t size);
void putchar(
    uint32_t c,
    uint32_t cx, uint32_t cy,
    uint32_t fg, uint32_t bg, char *psf_start, char *fb, uint32_t scanline);

#endif
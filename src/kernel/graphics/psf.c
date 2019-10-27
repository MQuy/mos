#include <kernel/memory/malloc.h>
#include <kernel/include/ctype.h>
#include <kernel/include/string.h>
#include <kernel/include/common.h>
#include "psf.h"

uint16_t *unicode;

void psf_init(char *psf_start, size_t size)
{
  uint16_t glyph = 0;
  char *psf_end = psf_start + size;
  /* cast the address to PSF header struct */
  psf_t *font = (psf_t *)psf_start;
  /* get the offset of the table */
  char *s = (char *)((char *)psf_start +
                     font->headersize +
                     font->numglyph * font->bytesperglyph);
  /* is there a unicode table? */
  if (font->flags & PSF_HAS_UNICODE_TABLE & s < psf_end)
  {
    /* allocate memory for translation table */
    unicode = malloc(USHRT_MAX * sizeof(uint16_t));
    memset(unicode, 0, USHRT_MAX * sizeof(uint16_t));
    // decode translation table
    while (s < psf_end && glyph < USHRT_MAX)
    {
      uint16_t c = (uint16_t)((uint8_t)s[0]);
      if (c == 0xFF)
      {
        // end of line, next glyph
        glyph++;
      }
      else
      {
        // store glyph index for this UTF-8 character
        if ((c & 128) != 0)
        {
          if ((c & 32) == 0)
          {
            c = ((s[0] & 0x1F) << 6) + (s[1] & 0x3F);
            s++;
          }
          else if ((c & 16) == 0)
          {
            c = ((((s[0] & 0xF) << 6) + (s[1] & 0x3F)) << 6) + (s[2] & 0x3F);
            s += 2;
          }
          else if ((c & 8) == 0)
          {
            c = ((((((s[0] & 0x7) << 6) + (s[1] & 0x3F)) << 6) + (s[2] & 0x3F)) << 6) + (s[3] & 0x3F);
            s += 3;
          }
          else
            c = 0;
        }
        if (c < USHRT_MAX)
          unicode[c] = glyph;
      }
      s++;
    };
  }
  else
  {
    unicode = NULL;
  }
}

void putchar(
    /* note that this is int, not char as it's a unicode character */
    uint32_t c,
    /* cursor position on screen, in characters not in pixels */
    uint32_t cx, uint32_t cy,
    /* foreground and background colors, say 0xFFFFFF and 0x000000 */
    uint32_t fg, uint32_t bg, char *psf_start, char *fb, uint32_t scanline)
{
  /* cast the address to PSF header struct */
  psf_t *font = (psf_t *)psf_start;
  /* we need to know how many bytes encode one row */
  int bytesperline = div_ceil(font->width, 8);
  /* unicode translation */
  if (unicode != NULL)
  {
    c = unicode[c];
  }
  /* get the glyph for the character. If there's no
       glyph for a given character, we'll display the first glyph. */
  unsigned char *glyph =
      (unsigned char *)psf_start +
      font->headersize +
      (c > 0 && c < font->numglyph ? c : 0) * font->bytesperglyph;
  /* calculate the upper left corner on screen where we want to display.
       we only do this once, and adjust the offset later. This is faster. */
  int offs =
      (cy * font->height * scanline) +
      (cx * (font->width + 1) * 4);
  /* finally display pixels according to the bitmap */
  int x, y, line, mask;
  for (y = 0; y < font->height; y++)
  {
    /* save the starting position of the line */
    line = offs;
    mask = 1 << (font->width - 1);
    /* display a row */
    for (x = 0; x < font->width; x++)
    {
      *((uint32_t *)(fb + line)) = ((int)*glyph) & (mask) ? fg : bg;
      /* adjust to the next pixel */
      mask >>= 1;
      line += 4;
    }
    /* adjust to the next line */
    glyph += bytesperline;
    offs += scanline;
  }
}
#include "psf.h"

#include <include/limits.h>
#include <memory/vmm.h>
#include <utils/math.h>
#include <utils/string.h>

#define PSF_MAGIC 0x864ab572

static uint16_t *unicode;
static char *psf_start;

void psf_init(char *buff, size_t size)
{
	psf_start = buff;

	uint16_t glyph = 0;
	char *psf_end = psf_start + size;
	/* cast the address to PSF header struct */
	struct psf_t *font = (struct psf_t *)psf_start;
	/* get the offset of the table */
	char *s = (char *)((char *)psf_start +
					   font->headersize +
					   font->numglyph * font->bytesperglyph);
	/* is there a unicode table? */
	if (font->flags & PSF_HAS_UNICODE_TABLE && s < psf_end)
	{
		/* allocate memory for translation table */
		unicode = kcalloc(USHRT_MAX, sizeof(uint16_t));
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

void psf_putchar(
	/* note that this is int, not char as it's a unicode character */
	uint32_t c,
	/* cursor position on screen, in characters not in pixels */
	uint32_t cx, uint32_t cy,
	/* foreground and background colors, say 0xFFFFFF and 0x000000 */
	uint32_t fg, uint32_t bg, char *fb, uint32_t scanline)
{
	/* cast the address to PSF header struct */
	struct psf_t *font = (struct psf_t *)psf_start;
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
	int offs = cy * scanline + cx * 4;
	/* finally display pixels according to the bitmap */
	uint32_t x, y, line, mask;
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

void psf_puts(
	const char *s,
	uint32_t cx, uint32_t cy,
	uint32_t fg, uint32_t bg, char *fb, uint32_t scanline)
{
	struct psf_t *font = (struct psf_t *)psf_start;
	for (uint32_t i = 0, length = strlen(s); i < length; i++)
		psf_putchar(s[i], cx + i * font->width, cy, fg, bg, fb, scanline);
}

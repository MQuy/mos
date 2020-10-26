#ifndef LIBC_BMP_H
#define LIBC_BMP_H

#include <libgui/layout.h>
#include <stddef.h>
#include <stdint.h>

#define BI_RGB 0
#define BI_RLE8 1
#define BI_RLE4 2
#define BI_BITFIELDS 3
#define BI_JPEG 4
#define BI_PNG 5

#define BITMAPCOREHEADER 12
#define BITMAPCOREHEADER2 64
#define OS22XBITMAPHEADER 16
#define BITMAPINFOHEADER 40
#define BITMAPV2INFOHEADER 52
#define BITMAPV3INFOHEADER 56
#define BITMAPV4HEADER 108
#define BITMAPV5HEADER 124

struct bmp_header
{
	int16_t signature;
	uint32_t file_size;
	uint32_t reserved;
	uint32_t data_offset;
} __attribute__((packed));

struct dip_bitmapcoreheader
{
	uint32_t header_size;	  // this header's size
	int32_t width_px;		  // image width
	int32_t height_px;		  // image height
	uint16_t num_planes;	  // number of color planes
	uint16_t bits_per_pixel;  // bits per pixel
	uint32_t compression;
	uint32_t image_size_bytes;
};

// altered from https://engineering.purdue.edu/ece264/17au/hw/HW15
struct dip_bitmapinfoheader	 // 40 bytes, for total offset of 54 bytes
{
	uint32_t header_size;		// DIB header size in bytes (40 bytes)
	int32_t width_px;			// image width
	int32_t height_px;			// image height
	uint16_t num_planes;		// number of color planes
	uint16_t bits_per_pixel;	// bits per pixel
	uint32_t compression;		// compression type
	uint32_t image_size_bytes;	// image size (bytes)
	int32_t x_resolution_ppm;	// pixels per meter
	int32_t y_resolution_ppm;	// pixels per meter
	uint32_t num_colors;		// number of colors
	uint32_t important_colors;	// Important colors
};

// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-wmf/21164882-3cb3-49d6-8133-74396704f14d
struct ciexyz
{
	uint32_t ciexyzX;  // A 32-bit 2.30 fixed point type that defines the x chromaticity value.
	uint32_t ciexyzY;  // A 32-bit 2.30 fixed point type that defines the y chromaticity value.
	uint32_t ciexyzZ;  // A 32-bit 2.30 fixed point type that defines the z chromaticity value.
};

// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-wmf/07b63c50-c07b-44bc-9523-be4e90d20a8e
struct ciexyztriple
{
	struct ciexyz ciexyzRed;	// A 96-bit CIEXYZ Object that defines the red chromaticity values.
	struct ciexyz ciexyzGreen;	// A 96-bit CIEXYZ Object that defines the green chromaticity values.
	struct ciexyz ciexyzBlue;	// A 96-bit CIEXYZ Object that defines the blue chromaticity values.
} ciexyztriple;

// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv4header
struct dip_bitmapv4header  // 108 bytes, for total offset of 122 bytes
{
	uint32_t bV4Size;
	int32_t bV4Width;
	int32_t bV4Height;
	uint16_t bV4Planes;
	uint16_t bV4BitCount;
	uint32_t bV4V4Compression;
	uint32_t bV4SizeImage;
	int32_t bV4XPelsPerMeter;
	int32_t bV4YPelsPerMeter;
	uint32_t bV4ClrUsed;
	uint32_t bV4ClrImportant;
	uint32_t bV4RedMask;
	uint32_t bV4GreenMask;
	uint32_t bV4BlueMask;
	uint32_t bV4AlphaMask;
	uint32_t bV4CSType;
	struct ciexyztriple bV4Endpoints;
	uint32_t bV4GammaRed;
	uint32_t bV4GammaGreen;
	uint32_t bV4GammaBlue;
} dip_bitmapv4header;

// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv5header
struct dip_bitmapv5header  // 124 bytes, for a total offset of 138 bytes
{
	uint32_t bV5Size;
	int32_t bV5Width;
	int32_t bV5Height;
	uint16_t bV5Planes;
	uint16_t bV5BitCount;
	uint32_t bV5Compression;
	uint32_t bV5SizeImage;
	int32_t bV5XPelsPerMeter;
	int32_t bV5YPelsPerMeter;
	uint32_t bV5ClrUsed;
	uint32_t bV5ClrImportant;
	uint32_t bV5RedMask;
	uint32_t bV5GreenMask;
	uint32_t bV5BlueMask;
	uint32_t bV5AlphaMask;
	uint32_t bV5CSType;
	struct ciexyztriple bV5Endpoints;
	uint32_t bV5GammaRed;
	uint32_t bV5GammaGreen;
	uint32_t bV5GammaBlue;
	uint32_t bV5Intent;
	uint32_t bV5ProfileData;
	uint32_t bV5ProfileSize;
	uint32_t bV5Reserved;
};

void bmp_draw(struct graphic *dgraph, char *bmph, int32_t px, int32_t py);

#endif

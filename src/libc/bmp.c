#include "bmp.h"

void bmp_bi_bitfields_draw(
	struct graphic *dgraph,
	struct bmp_header *bmp_header,
	struct dip_bitmapcoreheader *dip_header,
	int32_t px,
	int32_t py)
{
	uint32_t row_size = dip_header->image_size_bytes / dip_header->height_px;
	char *bmp_data = (char *)bmp_header + bmp_header->data_offset;
	uint8_t bytes_per_pixel = dip_header->bits_per_pixel / 8;
	for (int32_t y = dip_header->height_px - 1; y >= 0; --y)
	{
		char *ibuf = dgraph->buf + (py + dip_header->height_px - y - 1) * dgraph->width * 4 + px * 4;
		char *iwin = bmp_data + y * row_size;
		for (int32_t x = 0; x < dip_header->width_px; ++x)
		{
			uint8_t alpha = bytes_per_pixel == 4 ? iwin[3] : 255;
			set_pixel(ibuf, iwin[0], iwin[1], iwin[2], alpha);
			iwin += bytes_per_pixel;
			ibuf += 4;
		}
	}
}

void bmp_draw(struct graphic *dgraph, char *bmph, int32_t px, int32_t py)
{
	struct bmp_header *bmp_header = (struct bmp_header *)bmph;
	struct dip_bitmapcoreheader *dip_header = (struct dip_bitmapcoreheader *)(bmph + 0xE);

	switch (dip_header->compression)
	{
	case BI_RGB:
	case BI_BITFIELDS:
		bmp_bi_bitfields_draw(dgraph, bmp_header, dip_header, px, py);
		break;

	default:
		break;
	}
}

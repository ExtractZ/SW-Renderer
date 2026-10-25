//framebuffer and 2D drawing
#ifndef GFX_H
#define GFX_H

#include <stdint.h>

/*
The renderer draws into gfx_fb (in RAM), then the whole buffer is sent to the
display in one DMA transfer. Pixels are RGB565, row by row from the top-left:
	gfx_fb[y * GFX_WIDTH + x]. This is to write the pixel in with specific color.

*/
#define GFX_WIDTH  240
#define GFX_HEIGHT 240
#define GFX_PIXELS (GFX_WIDTH * GFX_HEIGHT)

//8-bit-per-channel color -> RGB565 (5 bits red, 6 green, 5 blue)
#define GFX_RGB(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

#define GFX_BLACK  0x0000
#define GFX_WHITE  0xFFFF
#define GFX_RED    0xF800
#define GFX_GREEN  0x07E0
#define GFX_BLUE   0x001F

extern uint16_t gfx_fb[GFX_PIXELS];

void gfx_clear(uint16_t color);

//off-screen pixels are ignored, so lines can run partly off the edge
static inline void gfx_pixel(int32_t x, int32_t y, uint16_t color)
{
	//casting to unsigned turns negative values huge, so one compare covers both sides
	if ((uint32_t)x < GFX_WIDTH && (uint32_t)y < GFX_HEIGHT)
		gfx_fb[y * GFX_WIDTH + x] = color;
}

void gfx_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t color);

#endif

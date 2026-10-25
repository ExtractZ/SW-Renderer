#include "gfx.h"

//on the STM32 the framebuffer goes in SRAM3
#ifdef __arm__
#define FB_SECTION __attribute__((section(".ram3"), aligned(4)))
#else
#define FB_SECTION
#endif

uint16_t gfx_fb[GFX_PIXELS] FB_SECTION;

void gfx_clear(uint16_t color)
{
	//write two pixels per store: half as many writes as a uint16_t loop
	uint32_t two = ((uint32_t)color << 16) | color;
	uint32_t *p = (uint32_t *)gfx_fb;

	for (uint32_t i = 0; i < GFX_PIXELS / 2; i++)
		p[i] = two;
}

/*
Bresenham's line algorithm: steps one pixel at a time along the line using only
integer adds. err tracks how far the true line is from the pixel being drawn,
and decides each step whether to move in x, in y, or both.
*/
void gfx_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t color)
{
	int32_t dx = x1 > x0 ? x1 - x0 : x0 - x1;
	int32_t dy = y1 > y0 ? y0 - y1 : y1 - y0; //negative on purpose
	int32_t sx = x0 < x1 ? 1 : -1;
	int32_t sy = y0 < y1 ? 1 : -1;
	int32_t err = dx + dy;

	while (1) {
		gfx_pixel(x0, y0, color);
		if (x0 == x1 && y0 == y1) break;

		int32_t e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; } //step in x
		if (e2 <= dx) { err += dx; y0 += sy; } //step in y
	}
}

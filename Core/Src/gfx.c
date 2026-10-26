#include "gfx.h"

//on the STM32 the framebuffer goes in SRAM3
#ifdef __arm__
#define FB_SECTION __attribute__((section(".ram3"), aligned(4)))
#else
#define FB_SECTION
#endif

_Static_assert(GFX_HEIGHT % GFX_STRIP_ROWS == 0, "GFX_STRIP_ROWS must divide the screen height");

static uint16_t fb_a[GFX_STRIP_PIXELS] FB_SECTION;
static uint16_t fb_b[GFX_STRIP_PIXELS] FB_SECTION;
uint16_t *gfx_fb = fb_a; //back buffer: the one being drawn into

uint16_t *gfx_swap(void)
{
	uint16_t *done = gfx_fb;
	gfx_fb = (gfx_fb == fb_a) ? fb_b : fb_a;
	return done;
}
static uint16_t zb[GFX_STRIP_PIXELS] FB_SECTION; //z-buffer, also in SRAM3
uint16_t *gfx_zb = zb;

int32_t gfx_y0 = 0;
int32_t gfx_rows = GFX_STRIP_ROWS;

void gfx_begin_strip(int32_t y0, int32_t rows) //example (80,40) meaning row 80 to 119
{
	gfx_y0 = y0; //screen row where current strip starts
	gfx_rows = rows;
}

//clears zbuffer depth
void gfx_clear_depth(void)
{
	uint32_t *p = (uint32_t *)gfx_zb;
	for (int32_t i = 0; i < gfx_rows * GFX_WIDTH / 2; i++)
		p[i] = 0;
}

void gfx_clear(uint16_t color)
{
	//write two pixels per store: half as many writes as a uint16_t loop
	uint32_t two = ((uint32_t)color << 16) | color;
	uint32_t *p = (uint32_t *)gfx_fb;

	for (int32_t i = 0; i < gfx_rows * GFX_WIDTH / 2; i++)
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

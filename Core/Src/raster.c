#include "raster.h"
#include "gfx.h"

/*
rasterizer draws the triangles. it follows a set of rules:
1. triangle not facing = no drawing needed
2. bounding box to only look at pixels neat triangle
3. check if pixel center is in the triangle (for very small values like 100.4) 
*/
#define SUB_BITS 4
#define SUB_ONE  (1 << SUB_BITS)

//Q16.16 -> Q.4, rounded to the nearest 1/16 pixel
static inline int32_t to_sub(fx v)
{
	return (v + (1 << (FX_SHIFT - SUB_BITS - 1))) >> (FX_SHIFT - SUB_BITS);
}

static inline int32_t edge_bias(int32_t ax, int32_t ay, int32_t bx, int32_t by)
{
	int32_t dx = bx - ax, dy = by - ay;
	return (dy < 0 || (dy == 0 && dx > 0)) ? 0 : -1;
}

static inline int32_t min3(int32_t a, int32_t b, int32_t c) { int32_t m = a < b ? a : b; return m < c ? m : c; }
static inline int32_t max3(int32_t a, int32_t b, int32_t c) { int32_t m = a > b ? a : b; return m > c ? m : c; }

int raster_tri_flat(const vec3 *v0, const vec3 *v1, const vec3 *v2, uint16_t color)
{
	//reject anything far off-screen before converting (keeps the math in 32 bits)
	const fx lo = fx_from_int(-RASTER_GUARD);
	const fx hi_x = fx_from_int(GFX_WIDTH + RASTER_GUARD);
	const fx hi_y = fx_from_int(GFX_HEIGHT + RASTER_GUARD);
	if (v0->x < lo || v1->x < lo || v2->x < lo || v0->y < lo || v1->y < lo || v2->y < lo ||
	    v0->x > hi_x || v1->x > hi_x || v2->x > hi_x || v0->y > hi_y || v1->y > hi_y || v2->y > hi_y)
		return 0;

	int32_t x0 = to_sub(v0->x), y0 = to_sub(v0->y);
	int32_t x1 = to_sub(v1->x), y1 = to_sub(v1->y);
	int32_t x2 = to_sub(v2->x), y2 = to_sub(v2->y);

	//twice the signed area: positive = clockwise on screen = facing the camera
	//by tracing the triangle by hand, clockwise order of edges mean its front facing
	int32_t area = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
	if (area <= 0) return 0;

	//bounding box in whole pixels, clipped to the screen. reduces the rastrizer searching for pixels to fill for triangle
	int32_t min_x = min3(x0, x1, x2) >> SUB_BITS;
	int32_t min_y = min3(y0, y1, y2) >> SUB_BITS;
	int32_t max_x = max3(x0, x1, x2) >> SUB_BITS;
	int32_t max_y = max3(y0, y1, y2) >> SUB_BITS;
	if (min_x < 0) min_x = 0;
	if (min_y < 0) min_y = 0;
	if (max_x > GFX_WIDTH - 1) max_x = GFX_WIDTH - 1;
	if (max_y > GFX_HEIGHT - 1) max_y = GFX_HEIGHT - 1;
	if (min_x > max_x || min_y > max_y) return 0; //entirely off-screen

	//edge value changes depending on moving pixels
	//edge i is the one opposite vertex i (w0: v1->v2, w1: v2->v0, w2: v0->v1)
	//per-pixel steps: moving right subtracts dy, moving down adds dx (both scaled to whole pixels)
	int32_t step_x0 = -(y2 - y1) * SUB_ONE, step_y0 = (x2 - x1) * SUB_ONE;
	int32_t step_x1 = -(y0 - y2) * SUB_ONE, step_y1 = (x0 - x2) * SUB_ONE;
	int32_t step_x2 = -(y1 - y0) * SUB_ONE, step_y2 = (x1 - x0) * SUB_ONE;

	//edge values at the center of the top-left pixel of the box, with the fill rule bias
	int32_t px = (min_x << SUB_BITS) + SUB_ONE / 2;
	int32_t py = (min_y << SUB_BITS) + SUB_ONE / 2;
	int32_t row0 = (x2 - x1) * (py - y1) - (y2 - y1) * (px - x1) + edge_bias(x1, y1, x2, y2);
	int32_t row1 = (x0 - x2) * (py - y2) - (y0 - y2) * (px - x2) + edge_bias(x2, y2, x0, y0);
	int32_t row2 = (x1 - x0) * (py - y0) - (y1 - y0) * (px - x0) + edge_bias(x0, y0, x1, y1);

	uint16_t *row = &gfx_fb[min_y * GFX_WIDTH];
	for (int32_t y = min_y; y <= max_y; y++) {
		int32_t w0 = row0, w1 = row1, w2 = row2;

		for (int32_t x = min_x; x <= max_x; x++) {
			//inside when all three are >= 0, i.e. none has its sign bit set
			if ((w0 | w1 | w2) >= 0)
				row[x] = color;
			w0 += step_x0;
			w1 += step_x1;
			w2 += step_x2;
		}

		row0 += step_y0;
		row1 += step_y1;
		row2 += step_y2;
		row += GFX_WIDTH;
	}
	return 1;
}

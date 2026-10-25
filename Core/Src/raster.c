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

//64-bit -> 32-bit without wrapping
static inline int32_t clamp32(int64_t v)
{
	return v > INT32_MAX ? INT32_MAX : (v < INT32_MIN ? INT32_MIN : (int32_t)v);
}

static inline int32_t min3(int32_t a, int32_t b, int32_t c) { int32_t m = a < b ? a : b; return m < c ? m : c; }
static inline int32_t max3(int32_t a, int32_t b, int32_t c) { int32_t m = a > b ? a : b; return m > c ? m : c; }

//now set to goraud or flat brightness
static inline __attribute__((always_inline))
int raster_tri(const vec3 *v0, const vec3 *v1, const vec3 *v2, uint16_t color,
               fx b0, fx b1, fx b2, const int gouraud)
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

	//edge values at the center of the top-left pixel of the box
	int32_t px = (min_x << SUB_BITS) + SUB_ONE / 2;
	int32_t py = (min_y << SUB_BITS) + SUB_ONE / 2;
	int32_t e0 = (x2 - x1) * (py - y1) - (y2 - y1) * (px - x1);
	int32_t e1 = (x0 - x2) * (py - y2) - (y0 - y2) * (px - x2);
	int32_t e2 = (x1 - x0) * (py - y0) - (y1 - y0) * (px - x0);

	//with the fill rule bias, for the inside test
	int32_t row0 = e0 + edge_bias(x1, y1, x2, y2);
	int32_t row1 = e1 + edge_bias(x2, y2, x0, y0);
	int32_t row2 = e2 + edge_bias(x0, y0, x1, y1);

	//weighted mix of dpeth for the pixels in between the vertices
	int64_t d0 = v0->z, d1 = v1->z, d2 = v2->z;
	int32_t z_row    = clamp32((d0 * e0 + d1 * e1 + d2 * e2) / area);
	int32_t z_step_x = clamp32((d0 * step_x0 + d1 * step_x1 + d2 * step_x2) / area);
	int32_t z_step_y = clamp32((d0 * step_y0 + d1 * step_y1 + d2 * step_y2) / area);

	//Gouraud: the corner brightnesses are mixed across the triangle exactly like depth
	int32_t br_row = 0, br_step_x = 0, br_step_y = 0;
	uint32_t base_r = 0, base_g = 0, base_b = 0;
	if (gouraud) {
		int64_t l0 = b0, l1 = b1, l2 = b2; //b for different brightness levels
		br_row    = clamp32((l0 * e0 + l1 * e1 + l2 * e2) / area);
		br_step_x = clamp32((l0 * step_x0 + l1 * step_x1 + l2 * step_x2) / area);
		br_step_y = clamp32((l0 * step_y0 + l1 * step_y1 + l2 * step_y2) / area);

		//split the base color once per triangle, not per pixel
		base_r = (color >> 11) & 0x1F;
		base_g = (color >> 5) & 0x3F;
		base_b = color & 0x1F;
	}

	uint16_t *row = &gfx_fb[min_y * GFX_WIDTH];
	uint16_t *zrow = &gfx_zb[min_y * GFX_WIDTH];
	for (int32_t y = min_y; y <= max_y; y++) {
		int32_t w0 = row0, w1 = row1, w2 = row2;
		int32_t z = z_row;
		int32_t br = br_row;

		for (int32_t x = min_x; x <= max_x; x++) {
			//inside when all three are >= 0, i.e. none has its sign bit set
			if ((w0 | w1 | w2) >= 0) {
				//depth test: only draw if nearer (bigger) than what's already there
				uint16_t depth = (uint16_t)(z >> 8);
				if (depth > zrow[x]) {
					zrow[x] = depth;
					if (gouraud) {
						//brightness at this pixel as 0..256, then scale each channel by it.
						//only done for pixels that are actually drawn
						uint32_t s = br > 0 ? (uint32_t)br >> 8 : 0;
						row[x] = (uint16_t)(((base_r * s >> 8) << 11) | ((base_g * s >> 8) << 5) | (base_b * s >> 8));
					} else {
						row[x] = color;
					}
				}
			}
			w0 += step_x0;
			w1 += step_x1;
			w2 += step_x2;
			z += z_step_x;
			if (gouraud) br += br_step_x;
		}

		row0 += step_y0;
		row1 += step_y1;
		row2 += step_y2;
		z_row += z_step_y;
		if (gouraud) br_row += br_step_y;
		row += GFX_WIDTH;
		zrow += GFX_WIDTH;
	}
	return 1;
}

int raster_tri_flat(const vec3 *v0, const vec3 *v1, const vec3 *v2, uint16_t color)
{
	return raster_tri(v0, v1, v2, color, 0, 0, 0, 0);
}

int raster_tri_gouraud(const vec3 *v0, const vec3 *v1, const vec3 *v2,
                       fx b0, fx b1, fx b2, uint16_t color)
{
	return raster_tri(v0, v1, v2, color, b0, b1, b2, 1);
}

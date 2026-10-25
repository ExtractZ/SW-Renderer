//Mac-side tests for the triangle rasterizer
//build and run:  make -C host test
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raster.h"
#include "gfx.h"
#include "project.h"

static int failures = 0;
static uint8_t coverage[GFX_PIXELS]; //how many triangles drew each pixel

static void fail(const char *msg)
{
	printf("FAIL %s\n", msg);
	failures++;
}

static vec3 P(double x, double y)
{
	return vec3_make((fx)lround(x * 65536), (fx)lround(y * 65536), FX_ONE);
}

//draws one triangle into a cleared framebuffer and adds what it covered to coverage[]
static int draw_counted(vec3 a, vec3 b, vec3 c)
{
	gfx_clear(0);
	gfx_clear_depth();
	int drawn = raster_tri_flat(&a, &b, &c, 1);
	for (int i = 0; i < GFX_PIXELS; i++)
		coverage[i] += (uint8_t)gfx_fb[i];
	return drawn;
}

static void count_coverage(int *zero, int *once, int *more)
{
	*zero = *once = *more = 0;
	for (int i = 0; i < GFX_PIXELS; i++) {
		if (coverage[i] == 0) (*zero)++;
		else if (coverage[i] == 1) (*once)++;
		else (*more)++;
	}
}

//two triangles covering the whole screen: the diagonal has pixel centers exactly
//on the shared edge, so every pixel must be drawn exactly once
static void test_full_screen(void)
{
	int zero, once, more;
	memset(coverage, 0, sizeof(coverage));
	draw_counted(P(0, 0), P(240, 0), P(240, 240));
	draw_counted(P(0, 0), P(240, 240), P(0, 240));
	count_coverage(&zero, &once, &more);
	if (zero != 0 || more != 0) fail("full screen: every pixel exactly once");
	printf("full screen quad: %d gaps, %d double-drawn\n", zero, more);
}

//a fan of triangles around a random center, with random sub-pixel corners:
//shared edges at every angle, none of them aligned to the pixel grid
static void test_fan(void)
{
	int zero, once, more, total_more = 0;
	srand(1234);

	for (int trial = 0; trial < 20; trial++) {
		memset(coverage, 0, sizeof(coverage));
		double cx = 100 + (rand() % 4000) / 100.0, cy = 100 + (rand() % 4000) / 100.0;
		const int n = 24;
		vec3 center = P(cx, cy);

		for (int i = 0; i < n; i++) {
			//clockwise on screen (y down) = increasing angle
			double a0 = 2 * M_PI * i / n, a1 = 2 * M_PI * (i + 1) / n;
			double r0 = 60 + (i * 37 % 23), r1 = 60 + ((i + 1) % n * 37 % 23);
			vec3 p0 = P(cx + r0 * cos(a0), cy + r0 * sin(a0));
			vec3 p1 = P(cx + r1 * cos(a1), cy + r1 * sin(a1));
			draw_counted(center, p0, p1);
		}
		count_coverage(&zero, &once, &more);
		total_more += more;
		if (once < 10000) fail("fan: suspiciously few pixels covered");
	}
	if (total_more != 0) fail("fan: shared edges drawn twice");
	printf("fan of 24 triangles x 20 trials: %d double-drawn pixels\n", total_more);
}

//a triangle and the same triangle reversed: one is front-facing, the other must be skipped
static void test_backface(void)
{
	vec3 a = P(10, 10), b = P(200, 30), c = P(50, 180);
	memset(coverage, 0, sizeof(coverage));
	if (!draw_counted(a, b, c)) fail("backface: clockwise triangle should be drawn");
	if (draw_counted(a, c, b)) fail("backface: counter-clockwise triangle should be skipped");
	if (draw_counted(a, a, c)) fail("degenerate: zero-area triangle should be skipped");
}

//area check: a right triangle with legs 100 covers ~5000 pixels
static void test_area(void)
{
	int zero, once, more;
	memset(coverage, 0, sizeof(coverage));
	draw_counted(P(20, 20), P(120, 20), P(20, 120));
	count_coverage(&zero, &once, &more);
	if (abs(once - 5000) > 100) fail("area: pixel count far from 5000");
	printf("right triangle, legs 100: %d pixels (expected ~5000)\n", once);
}

//partly off-screen and far off-screen
static void test_clipping(void)
{
	memset(coverage, 0, sizeof(coverage));
	if (!draw_counted(P(-100, -100), P(400, 50), P(50, 400))) fail("offscreen: partly visible triangle should draw");
	if (draw_counted(P(-5000, 0), P(100, 0), P(0, 100))) fail("guard: far off-screen corner should be skipped");
	if (draw_counted(P(300, 300), P(400, 300), P(300, 400))) fail("offscreen: fully off-screen should draw nothing");
}

//screen point with a depth computed exactly like project_point (camera distance z)
static vec3 PZ(double x, double y, double z)
{
	vec3 v = P(x, y);
	v.z = (fx)((PROJ_DEPTH_K << (FX_SHIFT + 8)) / (fx)lround(z * 65536));
	return v;
}

static uint16_t pixel(int x, int y) { return gfx_fb[y * GFX_WIDTH + x]; }

//two overlapping triangles: the nearer one must win whichever is drawn first
static void test_depth_order(void)
{
	vec3 n0 = PZ(20, 20, 3), n1 = PZ(200, 20, 3), n2 = PZ(20, 200, 3);    //near, z = 3
	vec3 f0 = PZ(40, 40, 6), f1 = PZ(220, 40, 6), f2 = PZ(40, 220, 6);    //far, z = 6

	gfx_clear(0);
	gfx_clear_depth();
	raster_tri_flat(&f0, &f1, &f2, 2);
	raster_tri_flat(&n0, &n1, &n2, 1);
	if (pixel(60, 60) != 1) fail("depth: far then near, near should win");
	if (pixel(180, 60) != 2) fail("depth: far triangle should show where near doesn't cover");

	gfx_clear(0);
	gfx_clear_depth();
	raster_tri_flat(&n0, &n1, &n2, 1);
	raster_tri_flat(&f0, &f1, &f2, 2);
	if (pixel(60, 60) != 1) fail("depth: near then far, near should still win");
}

//a tilted triangle (z = 2 on the left, 6 on the right) crossing a flat one at z = 4.
//1/z goes 0.5 -> 0.1667 linearly across the screen, reaching 1/4 at 75% of the
//way, so the tilted one must win left of that and lose right of it
static void test_depth_slope(void)
{
	vec3 t0 = PZ(20, 20, 2), t1 = PZ(220, 20, 6), t2 = PZ(20, 220, 2);
	vec3 t3 = PZ(220, 220, 6);
	vec3 f0 = PZ(0, 0, 4), f1 = PZ(240, 0, 4), f2 = PZ(0, 240, 4), f3 = PZ(240, 240, 4);

	gfx_clear(0);
	gfx_clear_depth();
	raster_tri_flat(&f0, &f1, &f3, 2); //flat quad covering the screen
	raster_tri_flat(&f0, &f3, &f2, 2);
	raster_tri_flat(&t0, &t1, &t3, 1); //tilted quad
	raster_tri_flat(&t0, &t3, &t2, 1);

	int crossover = 20 + (int)(0.75 * 200); //x = 170
	if (pixel(crossover - 10, 120) != 1) fail("depth slope: tilted should win before the crossover");
	if (pixel(crossover + 10, 120) != 2) fail("depth slope: flat should win after the crossover");

	int seen = 0; //where the switch actually happens
	for (int x = 20; x < 220; x++)
		if (pixel(x, 120) == 2) { seen = x; break; }
	printf("depth crossover at x = %d (expected %d)\n", seen, crossover);
	if (abs(seen - crossover) > 1) fail("depth slope: crossover in the wrong place");
}

//Gouraud: full brightness everywhere must match flat exactly, and a triangle lit
//only at one corner must fade from that corner to the others
static void test_gouraud(void)
{
	vec3 a = PZ(20, 20, 3), b = PZ(220, 20, 3), c = PZ(20, 220, 3);

	gfx_clear(0);
	gfx_clear_depth();
	raster_tri_gouraud(&a, &b, &c, FX_ONE, FX_ONE, FX_ONE, GFX_WHITE);
	int mismatched = 0;
	for (int y = 0; y < GFX_HEIGHT; y++)
		for (int x = 0; x < GFX_WIDTH; x++)
			if (pixel(x, y) != 0 && pixel(x, y) != GFX_WHITE) mismatched++;
	if (mismatched) fail("gouraud: brightness 1 everywhere should give the plain color");

	gfx_clear(0);
	gfx_clear_depth();
	raster_tri_gouraud(&a, &b, &c, FX_ONE, 0, 0, GFX_WHITE); //lit only at corner a
	int near_a = pixel(22, 22) >> 11;  //red channel, 0..31
	int middle = pixel(86, 86) >> 11;  //about a third of the way from a
	int near_b = pixel(215, 22) >> 11;
	printf("gouraud red channel: near lit corner %d, middle %d, near dark corner %d\n", near_a, middle, near_b);
	if (near_a < 29) fail("gouraud: should be bright next to the lit corner");
	if (middle < 8 || middle > 13) fail("gouraud: should be about 1/3 brightness at the center");
	if (near_b > 2) fail("gouraud: should be dark next to the unlit corners");
}

int main(void)
{
	test_gouraud();
	test_full_screen();
	test_fan();
	test_backface();
	test_area();
	test_clipping();
	test_depth_order();
	test_depth_slope();

	if (failures == 0) printf("all raster tests passed\n");
	else printf("%d failure(s)\n", failures);
	return failures != 0;
}

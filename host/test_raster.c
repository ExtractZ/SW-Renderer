//Mac-side tests for the triangle rasterizer
//build and run:  make -C host test
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raster.h"
#include "gfx.h"

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

int main(void)
{
	test_full_screen();
	test_fan();
	test_backface();
	test_area();
	test_clipping();

	if (failures == 0) printf("all raster tests passed\n");
	else printf("%d failure(s)\n", failures);
	return failures != 0;
}

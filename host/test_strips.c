#include <stdio.h>
#include <string.h>
#include "gfx.h"
#include "render.h"

static uint16_t full[GFX_PIXELS];
static uint16_t strips[GFX_PIXELS];

static void draw_frame(uint16_t *out, int strip_rows, int frame, int gouraud)
{
	mat3 rot;
	mat3_rotate_xyz(&rot, 13 * frame, 21 * frame, 8 * frame);
	render_prepare(&mesh_torus, &rot, vec3_make(0, 0, FX(4.0)), GFX_RGB(240, 150, 40), gouraud, NULL);

	for (int y0 = 0; y0 < GFX_HEIGHT; y0 += strip_rows) {
		int rows = (y0 + strip_rows <= GFX_HEIGHT) ? strip_rows : GFX_HEIGHT - y0;
		gfx_begin_strip(y0, rows);
		gfx_clear(GFX_BLACK);
		gfx_clear_depth();
		render_draw();
		memcpy(&out[y0 * GFX_WIDTH], gfx_fb, (size_t)rows * GFX_WIDTH * sizeof(uint16_t));
	}
}

int main(void)
{
	static const int heights[] = { 1, 8, 24, 40, 48, 70, 120 };
	int failures = 0;
	render_set_light(vec3_make(FX(-1.0), FX(1.0), FX(-1.0)));

	for (int gouraud = 0; gouraud <= 1; gouraud++) {
		for (int frame = 0; frame < 100; frame += 7) {
			draw_frame(full, GFX_HEIGHT, frame, gouraud);
			for (unsigned h = 0; h < sizeof(heights) / sizeof(heights[0]); h++) {
				draw_frame(strips, heights[h], frame, gouraud);
				int diff = 0;
				for (int i = 0; i < GFX_PIXELS; i++)
					diff += full[i] != strips[i];
				if (diff) {
					printf("FAIL %s frame %d, %d-row strips: %d pixels differ\n",
					       gouraud ? "gouraud" : "flat", frame, heights[h], diff);
					failures++;
				}
			}
		}
	}

	if (failures == 0) printf("strips identical to full frame (7 strip heights x 15 frames x flat/gouraud)\n");
	else printf("%d failure(s)\n", failures);
	return failures != 0;
}

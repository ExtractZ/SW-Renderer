//triangle rasterizer: fills the pixels inside a screen-space triangle
#ifndef RASTER_H
#define RASTER_H

#include <stdint.h>
#include "vec.h"

/*
Vertices are screen positions from project_point(): x, y in Q16.16 pixels
(z is ignored for now). The triangle must be clockwise on screen (positive
area), which is what a camera-facing triangle looks like; anything else is
skipped, so this also does backface culling.

Triangles reaching further than RASTER_GUARD pixels off-screen are skipped
(keeps the integer math from overflowing). Proper clipping comes later.
*/
#define RASTER_GUARD 512

//returns 1 if the triangle was drawn, 0 if skipped (backfacing, too thin, or too far off-screen)
int raster_tri_flat(const vec3 *v0, const vec3 *v1, const vec3 *v2, uint16_t color);

#endif

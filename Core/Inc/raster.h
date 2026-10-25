//triangle rasterizer: fills the pixels inside a screen-space triangle
#ifndef RASTER_H
#define RASTER_H

#include <stdint.h>
#include "vec.h"

#define RASTER_GUARD 512

//returns 1 if the triangle was drawn, 0 if skipped (backfacing, too thin, or too far off-screen)
int raster_tri_flat(const vec3 *v0, const vec3 *v1, const vec3 *v2, uint16_t color);

#endif

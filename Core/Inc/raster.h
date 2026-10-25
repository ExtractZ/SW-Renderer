//triangle rasterizer: fills the pixels inside a screen-space triangle
#ifndef RASTER_H
#define RASTER_H

#include <stdint.h>
#include "vec.h"

#define RASTER_GUARD 512

//returns 1 if the triangle was drawn, 0 if skipped (backfacing, too thin, or too far off-screen)
int raster_tri_flat(const vec3 *v0, const vec3 *v1, const vec3 *v2, uint16_t color);

//smooth shading: b0..b2 = brightness (0..1) at each corner, blended across the triangle
//and applied to color. same depth test and return value as raster_tri_flat
int raster_tri_gouraud(const vec3 *v0, const vec3 *v1, const vec3 *v2,
                       fx b0, fx b1, fx b2, uint16_t color);

#endif

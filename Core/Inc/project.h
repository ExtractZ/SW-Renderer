//perspective projection: 3D camera-space point -> 2D screen pixel
#ifndef PROJECT_H
#define PROJECT_H

#include "vec.h"
#include "gfx.h"

#define PROJ_FOCAL 208
#define PROJ_NEAR  FX(0.1) //points closer than this (or behind the camera) are rejected

//writes the screen position into out (x, y as fx pixels, z = depth kept for later)
//returns 0 if the point is too close or behind the camera
static inline int project_point(vec3 p, vec3 *out)
{
	if (p.z < PROJ_NEAR) return 0;

	//x and z are both Q16.16, so x / z is a plain ratio; scaling by focal and
	//shifting up 16 keeps the answer in Q16.16 (sub-pixel precision for later)
	out->x = fx_from_int(GFX_WIDTH / 2) + (fx)(((int64_t)p.x * PROJ_FOCAL << FX_SHIFT) / p.z);
	out->y = fx_from_int(GFX_HEIGHT / 2) - (fx)(((int64_t)p.y * PROJ_FOCAL << FX_SHIFT) / p.z);
	out->z = p.z;
	return 1;
}

#endif

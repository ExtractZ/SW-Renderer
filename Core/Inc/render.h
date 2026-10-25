//draws a mesh: transform, project, cull, shade, rasterize
#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include "mat.h"
#include "mesh.h"

#define RENDER_MAX_VERTS 1024 //meshes with more vertices are skipped

typedef struct {
	uint32_t tris_drawn;
	uint32_t tris_culled;  //facing away from the camera (or too thin to cover a pixel)
	uint32_t tris_skipped; //a corner behind the camera or far off-screen (no clipping yet)
} render_stats;

//direction from the scene toward the light, in camera space (normalized inside)
void render_set_light(vec3 to_light);
//draws one model
void render_mesh_flat(const mesh *m, const mat3 *rot, vec3 pos, uint16_t default_color, render_stats *stats);

#endif

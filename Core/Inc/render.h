//draws a mesh: transform, project, cull, shade, rasterize
#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include "mat.h"
#include "mesh.h"

#define RENDER_MAX_VERTS 1024 //meshes with more vertices (or triangles) are skipped
#define RENDER_MAX_TRIS  1024

typedef struct {
	uint32_t tris_drawn;   //passed culling and sent to the rasterizer
	uint32_t tris_culled;  //facing away from the camera
	uint32_t tris_skipped; //a corner behind the camera (no clipping yet)
} render_stats;

//direction from the scene toward the light, in camera space (normalized inside)
void render_set_light(vec3 to_light);

/*
Drawing happens in two steps, so a frame can be drawn in strips:
	render_prepare  once per frame: transform, light and cull the model, and keep
	                the list of visible triangles
	render_draw     once per strip: rasterize those triangles into the current
	                strip (see gfx_begin_strip). triangles that don't reach it are skipped
The model is rotated by rot, then moved to pos (camera space: z = distance ahead).
gouraud = 1: smooth lighting from the mesh's vertex normals (flat if it has none).
stats may be NULL; otherwise the counts are added to it.
One model per frame: render_prepare replaces the previous list.
*/
void render_prepare(const mesh *m, const mat3 *rot, vec3 pos, uint16_t default_color, int gouraud, render_stats *stats);
void render_draw(void);

//both steps at once, into the current strip (simple use, and the tests)
void render_mesh_flat(const mesh *m, const mat3 *rot, vec3 pos, uint16_t default_color, render_stats *stats);
void render_mesh_gouraud(const mesh *m, const mat3 *rot, vec3 pos, uint16_t default_color, render_stats *stats);

#endif

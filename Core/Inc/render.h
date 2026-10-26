//draws a mesh: transform, project, cull, shade, rasterize
#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include "mat.h"
#include "mesh.h"

#define RENDER_MAX_VERTS 1024 //meshes with more vertices are skipped
#define RENDER_MAX_TRIS  1024 //most triangles a model can have

typedef struct {
	uint32_t tris_drawn;
	uint32_t tris_culled;  //facing away from the camera (or too thin to cover a pixel)
	uint32_t tris_skipped; //a corner behind the camera or far off-screen (no clipping yet)
} render_stats;

//direction from the scene toward the light, in camera space (normalized inside)
void render_set_light(vec3 to_light);

void render_prepare(const mesh *m, const mat3 *rot, vec3 pos, uint16_t default_color, int gouraud, render_stats *stats);
uint32_t render_draw(void); //returns how many triangles it rasterized
//draws one model
void render_mesh_flat(const mesh *m, const mat3 *rot, vec3 pos, uint16_t default_color, render_stats *stats);

//same, but smooth: lighting at each vertex (from the mesh's vertex normals), blended
//across each triangle. meshes without normals fall back to flat
void render_mesh_gouraud(const mesh *m, const mat3 *rot, vec3 pos, uint16_t default_color, render_stats *stats);

#endif

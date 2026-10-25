#include "render.h"
#include "project.h"
#include "raster.h"

#define AMBIENT FX(0.2) //minimum brightness, so faces turned away from the light aren't pure black

//per-frame scratch space: each vertex is transformed and projected once,
//then shared by every triangle that uses it
static vec3 cam_verts[RENDER_MAX_VERTS];    //camera space (for normals)
static vec3 screen_verts[RENDER_MAX_VERTS]; //screen space (for rasterizing)
static uint8_t vert_ok[RENDER_MAX_VERTS];   //0 = behind the camera

static vec3 light_dir = { 0, 0, -FX_ONE }; //default: light coming from the camera

void render_set_light(vec3 to_light)
{
	light_dir = vec3_normalize(to_light);
}

//scales each channel of an RGB565 color by a brightness from 0 to 1
static uint16_t shade(uint16_t color, fx brightness)
{
	uint32_t r = (color >> 11) & 0x1F;
	uint32_t g = (color >> 5) & 0x3F;
	uint32_t b = color & 0x1F;

	r = (r * (uint32_t)brightness) >> FX_SHIFT;
	g = (g * (uint32_t)brightness) >> FX_SHIFT;
	b = (b * (uint32_t)brightness) >> FX_SHIFT;
	return (uint16_t)((r << 11) | (g << 5) | b);
}

void render_mesh_flat(const mesh *m, const mat3 *rot, vec3 pos, uint16_t default_color, render_stats *stats)
{
	render_stats st = { 0 };
	if (m->num_verts > RENDER_MAX_VERTS) return;

	//1. vertices: rotate, move in front of the camera, project
	for (uint32_t i = 0; i < m->num_verts; i++) {
		cam_verts[i] = vec3_add(mat3_mul_vec(rot, m->verts[i]), pos);
		vert_ok[i] = (uint8_t)project_point(cam_verts[i], &screen_verts[i]);
	}

	//2. triangles
	for (uint32_t t = 0; t < m->num_tris; t++) {
		uint16_t i0 = m->tris[t][0], i1 = m->tris[t][1], i2 = m->tris[t][2];
		if (!(vert_ok[i0] && vert_ok[i1] && vert_ok[i2])) {
			st.tris_skipped++;
			continue;
		}

		const vec3 *s0 = &screen_verts[i0], *s1 = &screen_verts[i1], *s2 = &screen_verts[i2];

		//backface culling: clockwise on screen (positive area) means facing the camera.
		//checked here so hidden triangles skip the normal and shading math below
		int64_t area = (int64_t)(s1->x - s0->x) * (s2->y - s0->y) - (int64_t)(s1->y - s0->y) * (s2->x - s0->x);
		if (area <= 0) {
			st.tris_culled++;
			continue;
		}

		//flat shading: face normal from the camera-space corners, brightness from N . L
		vec3 c0 = cam_verts[i0];
		vec3 normal = vec3_normalize(vec3_cross(vec3_sub(cam_verts[i1], c0), vec3_sub(cam_verts[i2], c0)));
		fx diffuse = fx_max(vec3_dot(normal, light_dir), 0); //facing away from the light = no direct light
		//clamped: rounding can push it a hair over 1, which would overflow a color channel
		fx brightness = fx_min(AMBIENT + fx_mul(FX_ONE - AMBIENT, diffuse), FX_ONE);

		uint16_t base = m->tri_colors ? m->tri_colors[t] : default_color;
		if (raster_tri_flat(s0, s1, s2, shade(base, brightness)))
			st.tris_drawn++;
		else
			st.tris_culled++; //too thin to matter, or far off-screen
	}

	if (stats) {
		stats->tris_drawn += st.tris_drawn;
		stats->tris_culled += st.tris_culled;
		stats->tris_skipped += st.tris_skipped;
	}
}

#include "mesh.h"
#include "gfx.h"

static const vec3 cube_verts[8] = {
	{ -FX_ONE, -FX_ONE, -FX_ONE }, {  FX_ONE, -FX_ONE, -FX_ONE },
	{  FX_ONE,  FX_ONE, -FX_ONE }, { -FX_ONE,  FX_ONE, -FX_ONE },
	{ -FX_ONE, -FX_ONE,  FX_ONE }, {  FX_ONE, -FX_ONE,  FX_ONE },
	{  FX_ONE,  FX_ONE,  FX_ONE }, { -FX_ONE,  FX_ONE,  FX_ONE },
};

//2 triangles per face, counter-clockwise seen from outside
static const uint16_t cube_tris[12][3] = {
	{ 0, 3, 2 }, { 0, 2, 1 }, //z = -1
	{ 4, 5, 6 }, { 4, 6, 7 }, //z = +1
	{ 0, 4, 7 }, { 0, 7, 3 }, //x = -1
	{ 1, 2, 6 }, { 1, 6, 5 }, //x = +1
	{ 0, 1, 5 }, { 0, 5, 4 }, //y = -1
	{ 3, 7, 6 }, { 3, 6, 2 }, //y = +1
};

static const uint16_t cube_colors[12] = {
	GFX_RGB(230,  60,  60), GFX_RGB(230,  60,  60), //red
	GFX_RGB( 60, 200,  80), GFX_RGB( 60, 200,  80), //green
	GFX_RGB( 70, 110, 240), GFX_RGB( 70, 110, 240), //blue
	GFX_RGB(240, 200,  60), GFX_RGB(240, 200,  60), //yellow
	GFX_RGB(200,  80, 220), GFX_RGB(200,  80, 220), //purple
	GFX_RGB( 60, 210, 220), GFX_RGB( 60, 210, 220), //cyan
};

const mesh mesh_cube = {
	.verts = cube_verts,
	.tris = cube_tris,
	.tri_colors = cube_colors,
	.num_verts = 8,
	.num_tris = 12,
};

#include "mat.h"

void mat3_identity(mat3 *out)
{
	*out = (mat3){ {
		{ FX_ONE, 0, 0 },
		{ 0, FX_ONE, 0 },
		{ 0, 0, FX_ONE },
	} };
}

void mat3_mul(mat3 *out, const mat3 *a, const mat3 *b)
{
	mat3 r; //build in a temporary so out can be the same matrix as a or b

	//entry (row, col) = row of a dotted with column of b
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
			r.m[row][col] = fx_dot3(a->m[row][0], b->m[0][col],
			                        a->m[row][1], b->m[1][col],
			                        a->m[row][2], b->m[2][col]);
	*out = r;
}

void mat3_transpose(mat3 *out, const mat3 *a)
{
	mat3 r;

	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
			r.m[row][col] = a->m[col][row];
	*out = r;
}


void mat3_rotate_x(mat3 *out, fx_angle a)
{
	fx s = fx_sin(a), c = fx_cos(a);
	*out = (mat3){ {
		{ FX_ONE, 0,  0 },
		{ 0,      c, -s },
		{ 0,      s,  c },
	} };
}

void mat3_rotate_y(mat3 *out, fx_angle a)
{
	fx s = fx_sin(a), c = fx_cos(a);
	*out = (mat3){ {
		{  c, 0,      s },
		{  0, FX_ONE, 0 },
		{ -s, 0,      c },
	} };
}

void mat3_rotate_z(mat3 *out, fx_angle a)
{
	fx s = fx_sin(a), c = fx_cos(a);
	*out = (mat3){ {
		{ c, -s, 0 },
		{ s,  c, 0 },
		{ 0,  0, FX_ONE },
	} };
}

void mat3_rotate_xyz(mat3 *out, fx_angle ax, fx_angle ay, fx_angle az)
{
	mat3 rx, ry, rz;
	mat3_rotate_x(&rx, ax);
	mat3_rotate_y(&ry, ay);
	mat3_rotate_z(&rz, az);

	mat3_mul(out, &ry, &rx); //x first, then y
	mat3_mul(out, &rz, out); //then z
}

//3x3 rotation matrices in Q16.16 fixed point
#ifndef MAT_H
#define MAT_H

#include "vec.h"

/*
i.e. rotate with a mat3, then move with a plain vec3 add. Perspective is a
separate divide later. That's 9 multiplies per vertex instead of 16.

Matrices are passed by pointer (36 bytes) to avoid copying them around.
*/
typedef struct {
	fx m[3][3];
} mat3;

//v' = M * v
static inline vec3 mat3_mul_vec(const mat3 *a, vec3 v)
{
	return vec3_make(
		fx_dot3(a->m[0][0], v.x, a->m[0][1], v.y, a->m[0][2], v.z),
		fx_dot3(a->m[1][0], v.x, a->m[1][1], v.y, a->m[1][2], v.z),
		fx_dot3(a->m[2][0], v.x, a->m[2][1], v.y, a->m[2][2], v.z));
}

void mat3_identity(mat3 *out);

//out = a * b: the combined rotation "b first, then a". out may be the same as a or b
void mat3_mul(mat3 *out, const mat3 *a, const mat3 *b);

//for a rotation matrix the transpose is its inverse (undoes the rotation)
void mat3_transpose(mat3 *out, const mat3 *a);
void mat3_rotate_x(mat3 *out, fx_angle a);
void mat3_rotate_y(mat3 *out, fx_angle a);
void mat3_rotate_z(mat3 *out, fx_angle a);
void mat3_rotate_xyz(mat3 *out, fx_angle ax, fx_angle ay, fx_angle az);

#endif

//3D vectors in Q16.16 fixed point
#ifndef VEC_H
#define VEC_H

#include "fixed.h"

/*
A vec3 is a point (position) or a direction. Each component is an fx.
Small functions are static inline, like fixed.h.
*/
typedef struct {
	fx x, y, z;
} vec3;

static inline vec3 vec3_make(fx x, fx y, fx z)
{
	vec3 v = { x, y, z };
	return v;
}

static inline vec3 vec3_add(vec3 a, vec3 b) { return vec3_make(a.x + b.x, a.y + b.y, a.z + b.z); }
static inline vec3 vec3_sub(vec3 a, vec3 b) { return vec3_make(a.x - b.x, a.y - b.y, a.z - b.z); }
static inline vec3 vec3_neg(vec3 a) { return vec3_make(-a.x, -a.y, -a.z); }

static inline vec3 vec3_scale(vec3 a, fx s)
{
	return vec3_make(fx_mul(a.x, s), fx_mul(a.y, s), fx_mul(a.z, s));
}

static inline fx fx_dot3(fx a0, fx b0, fx a1, fx b1, fx a2, fx b2)
{
	int64_t sum = (int64_t)a0 * b0 + (int64_t)a1 * b1 + (int64_t)a2 * b2;
	return (fx)(sum >> FX_SHIFT);
}

static inline fx vec3_dot(vec3 a, vec3 b)
{
	return fx_dot3(a.x, b.x, a.y, b.y, a.z, b.z);
}

static inline vec3 vec3_cross(vec3 a, vec3 b)
{
	return vec3_make(
		(fx)(((int64_t)a.y * b.z - (int64_t)a.z * b.y) >> FX_SHIFT),
		(fx)(((int64_t)a.z * b.x - (int64_t)a.x * b.z) >> FX_SHIFT),
		(fx)(((int64_t)a.x * b.y - (int64_t)a.y * b.x) >> FX_SHIFT));
}

fx vec3_length(vec3 v);
vec3 vec3_normalize(vec3 v); //same direction, length 1. a zero vector is returned unchanged

#endif

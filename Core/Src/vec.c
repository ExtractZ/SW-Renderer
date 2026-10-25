#include "vec.h"

fx vec3_length(vec3 v)
{
	uint64_t sq = (uint64_t)((int64_t)v.x * v.x)
	            + (uint64_t)((int64_t)v.y * v.y)
	            + (uint64_t)((int64_t)v.z * v.z);
	return (fx)fx_isqrt64(sq);
}

vec3 vec3_normalize(vec3 v)
{
	fx len = vec3_length(v);
	if (len == 0) return v; //a zero vector has no direction

	return vec3_make(fx_div(v.x, len), fx_div(v.y, len), fx_div(v.z, len));
}

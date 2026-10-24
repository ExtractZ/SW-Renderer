//Q16.16 fixed-point math for the renderer
#ifndef FIXED_H
#define FIXED_H

#include <stdint.h>

typedef int32_t fx;

#define FX_SHIFT 16
#define FX_ONE   ((fx)1 << FX_SHIFT)
#define FX_HALF  (FX_ONE >> 1)
#define FX_MAX   INT32_MAX
#define FX_MIN   INT32_MIN

//inline for pasting code, not jumping to it
#define FX(x) ((fx)((x) * 65536.0 + ((x) >= 0 ? 0.5 : -0.5)))

//conversions
static inline fx fx_from_int(int32_t i) { return (fx)((uint32_t)i << FX_SHIFT); }
static inline int32_t fx_to_int(fx a) { return a >> FX_SHIFT; } //rounds toward -infinity (floor)
static inline int32_t fx_round(fx a) { return (a + FX_HALF) >> FX_SHIFT; }
static inline float fx_to_float(fx a) { return (float)a / 65536.0f; }
static inline fx fx_frac(fx a) { return a & (FX_ONE - 1); } //fractional part

//add and subtract are plain + and -. multiply and divide need care:

//(a*65536) * (b*65536) = a*b*65536^2. 64 to prevent overflow
static inline fx fx_mul(fx a, fx b)
{
	return (fx)(((int64_t)a * b) >> FX_SHIFT);
}

//(a*65536) / (b*65536)
static inline fx fx_div(fx a, fx b)
{
	if (b == 0) return a >= 0 ? FX_MAX : FX_MIN; //saturate instead of crashing
	return (fx)(((int64_t)a << FX_SHIFT) / b);
}

static inline fx fx_abs(fx a) { return a < 0 ? -a : a; }
static inline fx fx_min(fx a, fx b) { return a < b ? a : b; }
static inline fx fx_max(fx a, fx b) { return a > b ? a : b; }
static inline fx fx_clamp(fx a, fx lo, fx hi) { return a < lo ? lo : (a > hi ? hi : a); }

fx fx_sqrt(fx a); //returns 0 for a <= 0
uint32_t fx_isqrt64(uint64_t n); //integer sqrt of a plain 64-bit number, rounded down

//the following is an invention of how to do angles: using 4096 as full 360. one step is 0.088 (260/4096)
typedef int32_t fx_angle;

#define FX_ANGLE_FULL    4096
#define FX_ANGLE_MASK    (FX_ANGLE_FULL - 1)
#define FX_ANGLE_QUARTER (FX_ANGLE_FULL / 4)
#define FX_DEG(d)        ((fx_angle)((d) * FX_ANGLE_FULL / 360)) //compile-time degrees -> angle
//extern means defined in another file
extern const fx fx_sin_table[FX_ANGLE_QUARTER + 1]; //quarter wave, fixed_sin_table.c

fx fx_sin(fx_angle a);
fx fx_cos(fx_angle a);

#endif

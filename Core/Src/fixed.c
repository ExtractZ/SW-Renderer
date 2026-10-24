#include "fixed.h"

fx fx_sin(fx_angle a)
{
	uint32_t wrapped = (uint32_t)a & FX_ANGLE_MASK;
	uint32_t quadrant = wrapped / FX_ANGLE_QUARTER;
	uint32_t i = wrapped % FX_ANGLE_QUARTER;

	switch (quadrant) {
	case 0:  return  fx_sin_table[i];
	case 1:  return  fx_sin_table[FX_ANGLE_QUARTER - i];
	case 2:  return -fx_sin_table[i];
	default: return -fx_sin_table[FX_ANGLE_QUARTER - i];
	}
}

//cos(x) = sin(x + 90 degrees)
fx fx_cos(fx_angle a)
{
	return fx_sin(a + FX_ANGLE_QUARTER);
}

fx fx_sqrt(fx a)
{
	if (a <= 0) return 0;

	uint64_t n = (uint64_t)a << FX_SHIFT;
	uint64_t result = 0;
	uint64_t bit = (uint64_t)1 << 62; //highest power of 4 that fits

	while (bit > n)
		bit >>= 2;

	while (bit != 0) {
		if (n >= result + bit) {
			n -= result + bit;
			result = (result >> 1) + bit;
		} else {
			result >>= 1;
		}
		bit >>= 2;
	}
	return (fx)result;
}

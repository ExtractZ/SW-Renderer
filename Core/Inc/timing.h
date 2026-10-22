//DWT cycle-counter setup and cycle_to_us() helper
#ifndef TIMING_H
#define TIMING_H

#include "main.h"
#include <stdint.h>

void Timing_Init(void); //enables the DWT cycle counter. call once at startup
uint32_t cycle_to_us(uint32_t cycles); //converts a cycle count to microseconds

//current cycle count. inline so reading it costs almost nothing
static inline uint32_t timing_cycles(void)
{
	return DWT->CYCCNT;
}

#endif

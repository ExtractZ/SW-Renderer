//do DWT cycle-counter setup and cycle_to_us() helper,
#ifndef __TIMING_H
#define __TIMING_H

#include "main.h"

void Timing_Init(void);
uint32_t cycle_to_us(uint32_t cycles);

#endif
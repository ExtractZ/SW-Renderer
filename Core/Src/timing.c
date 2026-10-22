#include "timing.h"

/*
The DWT (Data Watchpoint and Trace) unit is part of the Cortex-M4 debug hardware.
Its CYCCNT register counts every CPU clock cycle, so at 120 MHz one tick = 1/120 us.
*/

void Timing_Init(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; //trace enables in debug register
	DWT->CYCCNT = 0; //CYCCNT is the counter, 32bit, increases by 1 for every cpu clk cycle
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; //start counting
}

uint32_t cycle_to_us(uint32_t cycles)
{
	return cycles / (SystemCoreClock / 1000000u);
	//SystemCoreCLock holds the CPU frequency
}

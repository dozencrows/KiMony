/*
 * systick.c
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */
#include "systick.h"
#include "MKL26Z4.h"

volatile uint32_t sysTickCounter = 0;

void SysTick_Handler()
{
	sysTickCounter++;
}

#define CLOCK_HZ_TO_CLOCKS_PER_MS(x) (x / 1000)

uint32_t sysTickClocksPerMs = CLOCK_HZ_TO_CLOCKS_PER_MS(DEFAULT_SYSTEM_CLOCK);
uint32_t sysTickConfig = 0;

void systickSetClockRate(unsigned int clock_hz)
{
	sysTickClocksPerMs = CLOCK_HZ_TO_CLOCKS_PER_MS(clock_hz);
	if (clock_hz > 0xffffff) {
		sysTickConfig = 0;
		sysTickClocksPerMs /= 16;
	}
	else {
		sysTickConfig = 4;
	}
}

void systickDelayMs(unsigned int delayMs)
{
	SysTick->CTRL = 0x00U;
	sysTickCounter = 0;
	SysTick->LOAD = sysTickClocksPerMs * delayMs;
	SysTick->VAL = 0;
	SysTick->CTRL = 0x03U | sysTickConfig;

	while (!sysTickCounter) {
	  __asm("wfi");
	}

	SysTick->CTRL = 0x00U;
}

void systickEventInMs(unsigned int delayMs)
{
	SysTick->CTRL = 0x00U;
	sysTickCounter = 0;
	SysTick->LOAD = sysTickClocksPerMs * delayMs;
	SysTick->VAL = 0;
	SysTick->CTRL = 0x03U | sysTickConfig;
}

int systickCheckEvent()
{
	if (sysTickCounter) {
		SysTick->CTRL = 0x00U;
		return 1;
	}
	else {
		return 0;
	}
}

const uint32_t sysTickMax = 0xffffff;

void systickStartCycleCount()
{
	SysTick->CTRL = 0x00U;
	sysTickCounter = 0;

	SysTick->LOAD = sysTickMax;
	SysTick->VAL = 0;
	SysTick->CTRL = 0x07;
}

uint32_t systickStopCycleCount()
{
	SysTick->CTRL = 0x00U;
	return (sysTickCounter << 24) | (sysTickMax + 1 - SysTick->VAL);
}




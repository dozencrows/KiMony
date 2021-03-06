//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * systick.c
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */
#include "systick.h"

volatile uint32_t sysTickCounter = 0;

void SysTick_Handler()
{
    sysTickCounter++;
}

uint32_t sysTickClocksPerMs = DEFAULT_CLOCKS_PER_MS;
uint32_t sysTickConfig = 0;

void sysTickInit()
{
    NVIC_SetPriority(SysTick_IRQn, 0x80);
}

void sysTickSetClockRate(unsigned int clock_hz)
{
    sysTickClocksPerMs = CLOCK_HZ_TO_CLOCKS_PER_MS(clock_hz);
    if (clock_hz > 0xffffff) {
        sysTickConfig = 0;
        sysTickClocksPerMs /= 16;
    } else {
        sysTickConfig = 4;
    }
}

void sysTickDelayMs(unsigned int delayMs)
{
    uint32_t delayCycles = sysTickClocksPerMs * delayMs;
    uint32_t intTicks = (delayCycles & 0xff000000) >> 24;

    if (intTicks) {
        SysTick->CTRL = 0x00U;
        sysTickCounter = 0;
        SysTick->LOAD = 0xffffff;
        SysTick->VAL = 0;
        SysTick->CTRL = 0x03U | sysTickConfig;

        while (sysTickCounter < intTicks) {
            __asm("wfi");
        }
    }

    SysTick->CTRL = 0x00U;
    sysTickCounter = 0;
    SysTick->LOAD = delayCycles & 0xffffff;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x03U | sysTickConfig;

    while (!sysTickCounter) {
        __asm("wfi");
    }

    SysTick->CTRL = 0x00U;
}

void sysTickEventInMs(unsigned int delayMs)
{
    SysTick->CTRL = 0x00U;
    sysTickCounter = 0;
    SysTick->LOAD = sysTickClocksPerMs * delayMs;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x03U | sysTickConfig;
}

int sysTickCheckEvent()
{
    if (sysTickCounter) {
        SysTick->CTRL = 0x00U;
        return 1;
    } else {
        return 0;
    }
}

const uint32_t sysTickMax = 0xffffff;

void sysTickStartCycleCount()
{
    SysTick->CTRL = 0x00U;
    sysTickCounter = 0;

    SysTick->LOAD = sysTickMax;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x07;
}

uint32_t sysTickGetCycleCount()
{
    return (sysTickCounter << 24) | (sysTickMax + 1 - SysTick->VAL);
}

uint32_t sysTickStopCycleCount()
{
    SysTick->CTRL = 0x00U;
    return (sysTickCounter << 24) | (sysTickMax + 1 - SysTick->VAL);
}


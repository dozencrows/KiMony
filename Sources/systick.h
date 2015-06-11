//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * systick.h
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_
#include <stdint.h>
#include "MKL26Z4.h"

#define CLOCK_HZ_TO_CLOCKS_PER_MS(x) (x / 1000)
#define DEFAULT_CLOCKS_PER_MS (CLOCK_HZ_TO_CLOCKS_PER_MS(DEFAULT_SYSTEM_CLOCK))

extern void sysTickInit();
extern void sysTickSetClockRate(unsigned int clock_hz);
extern void sysTickDelayMs(unsigned int delayMs);
extern void sysTickEventInMs(unsigned int delayMs);
extern int sysTickCheckEvent();
extern void sysTickStartCycleCount();
extern uint32_t sysTickGetCycleCount();
extern uint32_t sysTickStopCycleCount();

#endif /* SYSTICK_H_ */

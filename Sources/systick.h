/*
 * systick.h
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_
#include <stdint.h>

extern void systickSetClockRate(unsigned int clock_hz);
extern void systickDelayMs(unsigned int delayMs);
extern void systickEventInMs(unsigned int delayMs);
extern int systickCheckEvent();
extern void systickStartCycleCount();
extern uint32_t systickStopCycleCount();

#endif /* SYSTICK_H_ */

/*
 * systick.h
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_
#include <stdint.h>

extern void sysTickInit();
extern void sysTickSetClockRate(unsigned int clock_hz);
extern void sysTickDelayMs(unsigned int delayMs);
extern void sysTickEventInMs(unsigned int delayMs);
extern int sysTickCheckEvent();
extern void sysTickStartCycleCount();
extern uint32_t sysTickGetCycleCount();
extern uint32_t sysTickStopCycleCount();

#endif /* SYSTICK_H_ */

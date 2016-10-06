/*
 * timer.h
 *
 *  Created on: 11 Jun 2015
 *      Author: ntuckett
 */

#ifndef TIMER_H_
#define TIMER_H_
#include <stdint.h>
#include "MKL26Z4.h"

#define TPM_CLOCKS_PER_MILLISECOND (DEFAULT_SYSTEM_CLOCK / 1000)	// Assumes TPM clock is PLL/2

extern void tpmInit();
extern void tpmStartTimer(int timerIndex, uint32_t periodClocks, uint32_t prescaleShift);
void tpmOneShotTimer(int timerIndex, uint32_t periodClocks, uint32_t prescaleShift);
extern uint32_t tpmGetTime(int timerIndex);
extern uint32_t tpmGetTimeHighPrecision(int timerIndex);
extern void tpmStopTimer(int timerIndex);

#endif /* TIMER_H_ */

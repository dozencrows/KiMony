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

// Define this to configure and use the fast internal reference clock to drive TPM
// at 4MHz
#define TPM_CLOCK_IRCLK

#if defined(TPM_CLOCK_IRCLK)
#define TPM_CLOCKS_PER_MILLISECOND 4000
#else
#define TPM_CLOCKS_PER_MILLISECOND (DEFAULT_SYSTEM_CLOCK / 1000)	// Assumes TPM clock is PLL/2
#endif

extern void tpmInit();
extern void tpmEnableTimer(int timerIndex);
extern void tpmStartTimer(int timerIndex, uint32_t periodClocks, uint32_t prescaleShift);
void tpmOneShotTimer(int timerIndex, uint32_t periodClocks, uint32_t prescaleShift);
extern uint32_t tpmGetTime(int timerIndex);
extern uint32_t tpmGetTimeHighPrecision(int timerIndex);
extern void tpmStopTimer(int timerIndex);
extern void tpmDisableTimer(int timerIndex);

#endif /* TIMER_H_ */

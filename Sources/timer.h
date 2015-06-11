/*
 * timer.h
 *
 *  Created on: 11 Jun 2015
 *      Author: ntuckett
 */

#ifndef TIMER_H_
#define TIMER_H_
#include <stdint.h>

extern void tpmInit();
extern void tpmStartMillisecondTimer(int timerIndex);
extern uint32_t tpmGetTime(int timerIndex);
extern void tpmStopTimer(int timerIndex);

#endif /* TIMER_H_ */

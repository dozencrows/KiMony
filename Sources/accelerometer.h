/*
 * accelerometer.h
 *
 *  Created on: 25 Jun 2015
 *      Author: ntuckett
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

extern void accelInit();
extern int accelCheckTransientInterrupt();
extern void accelClearInterrupts();

#endif /* ACCELEROMETER_H_ */
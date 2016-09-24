/*
 * accelerometer.h
 *
 *  Created on: 25 Jun 2015
 *      Author: ntuckett
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#define ACCEL_INTERRUPT_IGNORED		0
#define ACCEL_INTERRUPT_RECOGNISED	1

extern void accelInit();
extern int accelCheckTransientInterrupt();
extern int accelProcessInterrupt();

#endif /* ACCELEROMETER_H_ */

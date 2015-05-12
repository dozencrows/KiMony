/*
 * touchscreen.h
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#ifndef TOUCHSCREEN_H_
#define TOUCHSCREEN_H_

typedef struct _Point Point;

extern void touchScreenInit();
extern void touchScreenCalibrate();
extern void touchScreenTest();
extern int	touchScreenGetCoordinates(Point* p);

#endif /* TOUCHSCREEN_H_ */

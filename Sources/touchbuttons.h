/*
 * touchbuttons.h
 *
 *  Created on: 21 May 2015
 *      Author: ntuckett
 */

#ifndef TOUCHBUTTONS_H_
#define TOUCHBUTTONS_H_

#include <stdint.h>
#include "event.h"

typedef struct _IrAction IrAction;
typedef struct _Point Point;

typedef struct _TouchButton {
	const Event*	event;
	uint16_t		x, y;
	uint16_t		width, height;
	uint16_t		colour;
} TouchButton;

extern void touchbuttonsInit();
extern void touchbuttonsSetActive(const TouchButton* buttons, int count);
extern void touchbuttonsRender();
extern int touchbuttonsProcessTouch(const Point* touch, const Event** event);
extern void touchButtonsUpdate();

#endif /* TOUCHBUTTONS_H_ */

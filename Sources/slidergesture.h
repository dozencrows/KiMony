/*
 * slidergesture.h
 *
 *  Created on: 3 Sep 2016
 *      Author: ntuckett
 */

#ifndef SLIDERGESTURE_H_
#define SLIDERGESTURE_H_

#include <stdint.h>
#include "event.h"

typedef struct _GestureMapping {
	uint32_t	gesture;
	uint32_t	eventOffset;
} GestureMapping;

typedef enum {
	NONE 		= 0,
	TAP			= 1,
	DRAG_LEFT	= 2,
	DRAG_RIGHT	= 3,
	SWIPE_LEFT	= 4,
	SWIPE_RIGHT	= 5
} Gesture;

extern void sliderGestureInit();
extern void sliderGestureSetActiveMapping(const GestureMapping* mapping, int count);
extern int sliderGestureUpdate(uint32_t time, const Event** eventTriggered);
extern void sliderGestureFlush();

#endif /* SLIDERGESTURE_H_ */

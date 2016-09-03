/*
 * slidergesture.c
 *
 *  Created on: 3 Sep 2016
 *      Author: ntuckett
 */

#include "slidergesture.h"
#include <stdlib.h>
#include "capslider.h"
#include "flash.h"

typedef enum {
	IDLE,
	TOUCHING,
	DRAGGING
} GestureState;

typedef struct _SliderGesture {
	// Configuration
	uint8_t 		resolution;
	uint8_t			tapTimeThreshold;
	uint16_t		swipeThreshold;

	// State
	GestureState 	state;
	uint32_t		touchTime;
	uint8_t 		firstValue;
	uint8_t 		lastValue;
} SliderGesture;

static SliderGesture sliderGesture;

static void initSliderGesture(SliderGesture* sliderGesture)
{
	sliderGesture->resolution		= 20;
	sliderGesture->tapTimeThreshold = 20;
	sliderGesture->swipeThreshold	= 30;

	sliderGesture->state = IDLE;
	sliderGesture->firstValue	= 0;
	sliderGesture->lastValue	= 0;
}

static Gesture updateSliderGesture(SliderGesture* sliderGesture, uint8_t sliderValue, uint32_t time)
{
	int sliderTimeElapsed = time - sliderGesture->touchTime;
	Gesture result = NONE;

	switch (sliderGesture->state) {
		case IDLE: {
			if (sliderValue > 0) {
				sliderGesture->state 		= TOUCHING;
				sliderGesture->firstValue 	= sliderValue;
				sliderGesture->lastValue 	= sliderValue;
				sliderGesture->touchTime	= time;
			}
			break;
		}

		case TOUCHING: {
			if (sliderValue == 0) {
				int sliderDelta = sliderGesture->lastValue - sliderGesture->firstValue;

				sliderGesture->state = IDLE;
				sliderGesture->firstValue = 0;
				sliderGesture->lastValue = 0;

				if (abs(sliderDelta) <= sliderGesture->resolution && sliderTimeElapsed < sliderGesture->tapTimeThreshold) {
					result = TAP;
					break;
				} else {
					result = NONE;
					break;
				}
			}

			int sliderDelta = sliderGesture->lastValue - sliderGesture->firstValue;

			if (abs(sliderDelta) <= sliderGesture->resolution && sliderTimeElapsed < sliderGesture->tapTimeThreshold) {
				result = NONE;
				break;
			}

			sliderGesture->state = DRAGGING;
		}

		case DRAGGING: {
			if (sliderValue == 0) {
				sliderGesture->state = IDLE;
				sliderGesture->firstValue = 0;
				sliderGesture->lastValue = 0;
				result = NONE;
				break;
			}

			int sliderDelta = sliderValue - sliderGesture->firstValue;
			int sliderDeltaAbs = abs(sliderDelta);

			if (sliderDeltaAbs >= sliderGesture->swipeThreshold && sliderTimeElapsed < sliderGesture->tapTimeThreshold) {
				sliderGesture->state = IDLE;
				sliderGesture->firstValue = 0;
				sliderGesture->lastValue = 0;
				if (sliderDelta < 0) {
					result = SWIPE_LEFT;
				} else {
					result = SWIPE_RIGHT;
				}
				break;
			}
			if (sliderDeltaAbs >= sliderGesture->resolution) {
				if (sliderDelta < 0 && sliderGesture->firstValue > sliderGesture->resolution) {
					sliderGesture->firstValue -= sliderGesture->resolution;
					result = DRAG_LEFT;
					break;
				} else if (sliderGesture->firstValue < 100 - sliderGesture->resolution){
					sliderGesture->firstValue += sliderGesture->resolution;
					result = DRAG_RIGHT;
					break;
				}
			}

			break;
		}
	}

	sliderGesture->lastValue = sliderValue;
	return result;
}

static const GestureMapping* activeMapping = NULL;
static int activeMappingCount = 0;

void sliderGestureInit()
{
	capSliderInit();
	initSliderGesture(&sliderGesture);
}

void sliderGestureSetActiveMapping(const GestureMapping* mapping, int count)
{
	activeMapping = mapping;
	activeMappingCount = count;
}

int sliderGestureUpdate(uint32_t time, const Event** eventTriggered)
{
	int result = EVENT_NONE;
	uint8_t sliderValue = capSliderGetPercentage();
	Gesture gesture = updateSliderGesture(&sliderGesture, sliderValue, time);
	switch (gesture) {
		case NONE:
			break;
		case TAP:
		case DRAG_LEFT:
		case DRAG_RIGHT:
		case SWIPE_LEFT:
		case SWIPE_RIGHT:
			for (int i = 0; i < activeMappingCount; i++) {
				if (activeMapping[i].gesture == gesture) {
					*eventTriggered = (const Event*)GET_FLASH_PTR(activeMapping[i].eventOffset);
					result = (*eventTriggered)->type;
					break;
				}
			}
			break;
	}

	return result;
}

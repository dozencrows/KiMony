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
#include "debugutils.h"

#ifdef _DEBUG
static char s_history[16];
static int s_historyIndex = 0;

#define MAX_DEBUG_TT_LEN	15
#define DEBUG_TT_BEGIN s_historyIndex = 0
#define DEBUG_TT(x) if (s_historyIndex < MAX_DEBUG_TT_LEN) s_history[s_historyIndex++] = (x)
#define DEBUG_TT_END s_history[s_historyIndex] = '\0'
#else
#define DEBUG_TT_BEGIN
#define DEBUG_TT(x)
#define DEBUG_TT_END
#endif

typedef enum {
	IDLE,
	TOUCHING,
	DRAGGING,
	SETTLING,
} GestureState;

typedef struct _SliderGesture {
	// Configuration
	uint8_t 		resolution;
	uint8_t			tapTimeThreshold;
	uint8_t			swipeThreshold;
	uint8_t			swipeTimeThreshold;
	uint8_t			settleDelay;

	// State
	GestureState 	state;
	uint32_t		touchTime;
	uint8_t 		firstValue;
	uint8_t 		lastValue;
} SliderGesture;

static SliderGesture sliderGesture;

static void initSliderGesture(SliderGesture* sliderGesture)
{
	sliderGesture->resolution			= 20;	// units: percentage (delta threshold in one frame)
	sliderGesture->tapTimeThreshold 	= 20;	// units: time (roughly 100Hz base frequency)
	sliderGesture->swipeThreshold		= 30;	// units: percentage (delta threshold in one frame)
	sliderGesture->swipeTimeThreshold	= 40;	// units: time (roughly 100Hz base frequency)
	sliderGesture->settleDelay			= 40;	// units: time (roughly 100Hz base frequency)

	sliderGesture->state 		= IDLE;
	sliderGesture->touchTime	= 0;
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
				DEBUG_TT_BEGIN;

				sliderGesture->state 		= TOUCHING;
				sliderGesture->firstValue 	= sliderValue;
				sliderGesture->lastValue 	= sliderValue;
				sliderGesture->touchTime	= time;

				DEBUG_TT('t');
			}
			break;
		}

		case TOUCHING: {
			if (sliderValue == 0) {
				int sliderDelta = sliderGesture->lastValue - sliderGesture->firstValue;

				sliderGesture->state 		= SETTLING;
				sliderGesture->firstValue 	= 0;
				sliderGesture->lastValue 	= 0;
				sliderGesture->touchTime 	= time;

				if (abs(sliderDelta) <= sliderGesture->resolution && sliderTimeElapsed < sliderGesture->tapTimeThreshold) {
					result = TAP;
					DEBUG_TT('T');
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

			DEBUG_TT('D');
			sliderGesture->state 		= DRAGGING;
			sliderGesture->touchTime 	= time;
		}

		case DRAGGING: {
			if (sliderValue == 0) {
				sliderGesture->state 		= SETTLING;
				sliderGesture->firstValue 	= 0;
				sliderGesture->lastValue 	= 0;
				sliderGesture->touchTime 	= time;
				result = NONE;
				break;
			}

			int sliderDelta = sliderValue - sliderGesture->firstValue;
			int sliderDeltaAbs = abs(sliderDelta);

			if (sliderDeltaAbs >= sliderGesture->swipeThreshold && sliderTimeElapsed < sliderGesture->swipeTimeThreshold) {
				sliderGesture->state 		= SETTLING;
				sliderGesture->firstValue 	= 0;
				sliderGesture->lastValue 	= 0;
				sliderGesture->touchTime 	= time;
				if (sliderDelta < 0) {
					DEBUG_TT('L');
					result = SWIPE_LEFT;
				} else {
					DEBUG_TT('R');
					result = SWIPE_RIGHT;
				}
				break;
			}
			if (sliderDeltaAbs >= sliderGesture->resolution) {
				if (sliderDelta < 0 && sliderGesture->firstValue > sliderGesture->resolution) {
					sliderGesture->firstValue -= sliderGesture->resolution;
					DEBUG_TT('l');
					result = DRAG_LEFT;
					break;
				} else if (sliderGesture->firstValue < 100 - sliderGesture->resolution){
					sliderGesture->firstValue += sliderGesture->resolution;
					DEBUG_TT('r');
					result = DRAG_RIGHT;
					break;
				}
			}

			break;
		}

		case SETTLING: {
			if (sliderTimeElapsed > sliderGesture->settleDelay) {
				sliderGesture->state 		= IDLE;
				sliderGesture->touchTime 	= time;
			}
			break;
		}
	}

	sliderGesture->lastValue = sliderValue;

	DEBUG_TT_END;
	debugSetOverlayText(0, s_history);
	debugSetOverlayHex(1, sliderValue);
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

void sliderGestureFlush()
{
	DEBUG_TT_BEGIN;
	sliderGesture.state 		= IDLE;
	sliderGesture.touchTime		= 0;
	sliderGesture.firstValue	= 0;
	sliderGesture.lastValue		= 0;
}

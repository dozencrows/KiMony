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

#define SAMPLE_BUFFER_SIZE	300
static uint8_t s_sampleBuffer[SAMPLE_BUFFER_SIZE];
static int s_sampleBufferIndex = 0;

#define DEBUG_SB_CLEAR		s_sampleBufferIndex = 0
#define DEBUG_SB_SAMPLE(x)	if (s_sampleBufferIndex < SAMPLE_BUFFER_SIZE) s_sampleBuffer[s_sampleBufferIndex++] = (x);
#define DEBUG_SB_UPDATE		debugSetOverlayHex(2, s_sampleBufferIndex)
#else
#define DEBUG_TT_BEGIN
#define DEBUG_TT(x)
#define DEBUG_TT_END

#define DEBUG_SB_CLEAR
#define DEBUG_SB_SAMPLE(x)
#define DEBUG_SB_UPDATE
#endif

#ifdef _DEBUG
#include "renderer.h"
static void renderGesture() {
	if (s_sampleBufferIndex > 0) {
		rendererClearScreen();
		rendererNewDrawList();
		for (uint16_t y = 0; y < s_sampleBufferIndex; y++) {
			rendererDrawHLine(s_sampleBuffer[y], y, 1, 0xffff);
		}
	}
}
#define DEBUG_RENDER_GESTURE renderGesture()
#else
#define DEBUG_RENDER_GESTURE
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
	uint8_t			tapTimeMinThreshold;
	uint8_t			tapTimeMaxThreshold;
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
	sliderGesture->resolution			= 20;	// units: percentage (delta threshold from first touch point)
	sliderGesture->tapTimeMinThreshold 	= 5;	// units: time (roughly 100Hz base frequency)
	sliderGesture->tapTimeMaxThreshold 	= 15;	// units: time (roughly 100Hz base frequency)
	sliderGesture->settleDelay			= 40;	// units: time (roughly 100Hz base frequency)

	sliderGesture->state 		= IDLE;
	sliderGesture->touchTime	= 0;
	sliderGesture->firstValue	= 0;
	sliderGesture->lastValue	= 0;
}

static Gesture updateSliderGesture(SliderGesture* sliderGesture, uint8_t sliderValue, uint32_t time)
{
	Gesture result = NONE;

	switch (sliderGesture->state) {
		case IDLE: {
			if (sliderValue > 0) {
				DEBUG_TT_BEGIN;
				DEBUG_SB_CLEAR;

				sliderGesture->state 		= TOUCHING;
				sliderGesture->firstValue 	= sliderValue;
				sliderGesture->lastValue 	= sliderValue;
				sliderGesture->touchTime	= time;

				DEBUG_TT('t');
			} else {
				break;
			}
		}

		default: {
			DEBUG_SB_SAMPLE(sliderValue);
			int sliderTimeElapsed = time - sliderGesture->touchTime;

			switch (sliderGesture->state) {
				case TOUCHING: {
					if (sliderValue == 0) {
						int sliderDelta = sliderGesture->lastValue - sliderGesture->firstValue;

						sliderGesture->state 		= SETTLING;
						sliderGesture->firstValue 	= 0;
						sliderGesture->lastValue 	= 0;
						sliderGesture->touchTime 	= time;

						if (sliderTimeElapsed < sliderGesture->tapTimeMinThreshold) {
							result = NONE;
							break;
						} else if (abs(sliderDelta) <= sliderGesture->resolution) {
							result = TAP;
							DEBUG_TT('T');
							break;
						} else {
							if (sliderDelta < 0) {
								DEBUG_TT('L');
								result = SWIPE_LEFT;
							} else {
								DEBUG_TT('R');
								result = SWIPE_RIGHT;
							}
							break;
						}
					}

					if (sliderTimeElapsed < sliderGesture->tapTimeMaxThreshold) {
						result = TOUCH;
						break;
					}

					DEBUG_TT('D');
					sliderGesture->state = DRAGGING;
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

					result = TOUCH;
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
			break;
		}
	}

	sliderGesture->lastValue = sliderValue;

	DEBUG_TT_END;
	debugSetOverlayText(0, s_history);
//	debugSetOverlayHex(1, sliderValue);
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
		case TOUCH:
			result = EVENT_KEEPAWAKE;
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
			DEBUG_RENDER_GESTURE;
			break;
	}

	DEBUG_SB_UPDATE;
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

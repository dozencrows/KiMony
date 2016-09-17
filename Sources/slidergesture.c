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
static int32_t s_fitY0d;
static int32_t s_fitYNd;
static int32_t s_fitD;

#define DEBUG_SB_CLEAR		s_sampleBufferIndex = 0
#define DEBUG_SB_SAMPLE(x)	if (s_sampleBufferIndex < SAMPLE_BUFFER_SIZE) s_sampleBuffer[s_sampleBufferIndex++] = (x);
#define DEBUG_SB_UPDATE		debugSetOverlayHex(2, s_sampleBufferIndex)
#define DEBUG_SB_FIT(y0, yN, d)	s_fitY0d = y0; s_fitYNd = yN; s_fitD = d
#else
#define DEBUG_TT_BEGIN
#define DEBUG_TT(x)
#define DEBUG_TT_END

#define DEBUG_SB_CLEAR
#define DEBUG_SB_SAMPLE(x)
#define DEBUG_SB_UPDATE
#define DEBUG_SB_FIT(m, c, d)
#endif

#ifdef _DEBUG
#include "renderer.h"
static void renderGesture() {
	if (s_sampleBufferIndex > 0) {
		rendererClearScreen();
		rendererNewDrawList();
		for (uint16_t x = 0; x < s_sampleBufferIndex; x++) {
			rendererDrawHLine(s_sampleBuffer[x], x, 1, 0xffff);
		}

		int32_t y0 = s_fitY0d / s_fitD;
		int32_t yN = s_fitYNd / s_fitD;

		if (y0 < 0) {
			y0 = 0;
		} else if (y0 > SCREEN_WIDTH - 1) {
			y0 = SCREEN_WIDTH - 1;
		}

		if (yN < 0) {
			yN = 0;
		} else if (yN > SCREEN_WIDTH - 1) {
			yN = SCREEN_WIDTH - 1;
		}

		rendererDrawVLine(y0, 0, 2, 0xf800);
		rendererDrawVLine(yN, s_sampleBufferIndex - 1, 2, 0x07c0);
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
	uint8_t 		tapOffsetMaxThreshold;
	uint8_t			dragResolution;
	uint8_t			tapTimeMinThreshold;
	uint8_t			tapTimeMaxThreshold;
	uint8_t			settleDelay;

	// State
	GestureState 	state;
	uint32_t		touchTime;
	uint8_t 		firstValue;
	uint8_t 		lastValue;
	uint16_t		sampleCount;

	int32_t			sumX;
	int32_t			sumX2;
	int32_t			sumY;
	int32_t			sumY2;
	int32_t			sumXY;

} SliderGesture;

static SliderGesture sliderGesture;

static void initSliderGesture(SliderGesture* sliderGesture)
{
	sliderGesture->tapOffsetMaxThreshold	= 18;	// units: percentage (delta threshold from first touch point)
	sliderGesture->dragResolution			= 20;	// units: percentage (delta threshold from first touch point)
	sliderGesture->tapTimeMinThreshold 		= 5;	// units: time (roughly 100Hz base frequency)
	sliderGesture->tapTimeMaxThreshold 		= 15;	// units: time (roughly 100Hz base frequency)
	sliderGesture->settleDelay				= 40;	// units: time (roughly 100Hz base frequency)

	sliderGesture->state 		= IDLE;
	sliderGesture->touchTime	= 0;
	sliderGesture->firstValue	= 0;
	sliderGesture->lastValue	= 0;
	sliderGesture->sampleCount	= 0;

	sliderGesture->sumX 	= 0;
	sliderGesture->sumX2 	= 0;
	sliderGesture->sumY 	= 0;
	sliderGesture->sumY2 	= 0;
	sliderGesture->sumXY 	= 0;
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

				sliderGesture->sumX 	= 0;
				sliderGesture->sumX2 	= 0;
				sliderGesture->sumY 	= 0;
				sliderGesture->sumY2 	= 0;
				sliderGesture->sumXY 	= 0;
				sliderGesture->sampleCount	= 0;

				DEBUG_TT('t');
			} else {
				break;
			}
		}

		default: {
			DEBUG_SB_SAMPLE(sliderValue);
			int sliderTimeElapsed = time - sliderGesture->touchTime;
			sliderGesture->sampleCount++;

			switch (sliderGesture->state) {
				case TOUCHING: {
					if (sliderValue == 0) {
						sliderGesture->state 		= SETTLING;
						sliderGesture->firstValue 	= 0;
						sliderGesture->lastValue 	= 0;
						sliderGesture->touchTime 	= time;

						if (sliderTimeElapsed < sliderGesture->tapTimeMinThreshold) {
							result = NONE;
							break;
						}

						int32_t n = sliderGesture->sampleCount - 1;
						int32_t d = n * sliderGesture->sumX2 - sliderGesture->sumX * sliderGesture->sumX;
						int32_t m = n * sliderGesture->sumXY - sliderGesture->sumX * sliderGesture->sumY;
						int32_t c = sliderGesture->sumY * sliderGesture->sumX2 - sliderGesture->sumX * sliderGesture->sumXY;

						int32_t y0d = m + c;
						int32_t yNd = sliderTimeElapsed * m + c;
						DEBUG_SB_FIT(y0d, yNd, d);

						int32_t sliderDelta = (yNd - y0d) / d;

						if (abs(sliderDelta) <= sliderGesture->tapOffsetMaxThreshold) {
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
					} else if (sliderTimeElapsed < sliderGesture->tapTimeMaxThreshold) {
						sliderGesture->sumX 	+= sliderTimeElapsed;
						sliderGesture->sumX2	+= sliderTimeElapsed * sliderTimeElapsed;
						sliderGesture->sumY 	+= sliderValue;
						sliderGesture->sumY2	+= sliderValue * sliderValue;
						sliderGesture->sumXY	+= sliderValue * sliderTimeElapsed;
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

					if (sliderDeltaAbs >= sliderGesture->dragResolution) {
						if (sliderDelta < 0 && sliderGesture->firstValue > sliderGesture->dragResolution) {
							sliderGesture->firstValue -= sliderGesture->dragResolution;
							DEBUG_TT('l');
							result = DRAG_LEFT;
							break;
						} else if (sliderGesture->firstValue < 100 - sliderGesture->dragResolution){
							sliderGesture->firstValue += sliderGesture->dragResolution;
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

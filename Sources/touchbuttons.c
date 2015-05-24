/*
 * touchbuttons.c
 *
 *  Created on: 21 May 2015
 *      Author: ntuckett
 */
#include <stddef.h>
#include "touchbuttons.h"
#include "mathutil.h"
#include "renderer.h"
#include "fontdata.h"
#include "touchscreen.h"
#include "profiler.h"

#define MAX_BUTTONS				24

#define BUTTON_BORDER_COLOUR	0xffff
#define BUTTON_FLASH_COLOUR		0xffff
#define BUTTON_FLASH_COUNT		15

#define TOUCH_STATE_IDLE			0
#define TOUCH_STATE_PENDING			1
#define TOUCH_STATE_ACTIVE			2

#define TOUCH_DEBOUNCE_THRESHOLD	2

typedef struct _ButtonState {
	const TouchButton*	button;
	unsigned int dirty:1;
	unsigned int pressed:1;
	unsigned int counter:6;
} ButtonState;

static const TouchButton* activeTouchButtons = NULL;
static int activeTouchButtonsCount = 0;
static ButtonState buttonState[MAX_BUTTONS];
static int touchState = TOUCH_STATE_IDLE;
static int touchStateCounter = 0;
static int currentTouchButton = NULL;

static void renderTouchButton(const TouchButton* button, uint16_t colour)
{
	rendererDrawHLine(button->x, button->y, button->width, BUTTON_BORDER_COLOUR);
	rendererDrawVLine(button->x, button->y, button->height, BUTTON_BORDER_COLOUR);
	rendererDrawVLine(button->x + button->width - 1, button->y, button->height, BUTTON_BORDER_COLOUR);
	rendererDrawRect(button->x + 1, button->y + 1, button->width - 2, button->height - 2, colour);
	rendererDrawHLine(button->x, button->y + button->height - 1, button->width, BUTTON_BORDER_COLOUR);
	rendererDrawString("BTNX", button->x + 3, button->y + 3, &KiMony, 0xffff);
}

static int hitTestTouchButtons(const Point* touch)
{
	for(int i = 0; i < activeTouchButtonsCount; i++) {
		const TouchButton* button = buttonState[i].button;

		if (touch->x >= button->x && button->width > touch->x - button->x) {
			if (touch->y >= button->y && button->height > touch->y - button->y) {
				return i;
			}
		}
	}

	return -1;
}

static void setCurrentButtonPressedState(int pressed)
{
	if (currentTouchButton >= 0) {
		buttonState[currentTouchButton].pressed = pressed;
		buttonState[currentTouchButton].dirty = 1;
	}
}

void touchbuttonsInit()
{
}

void touchbuttonsSetActive(const TouchButton* buttons, int count)
{
	activeTouchButtons = buttons;
	activeTouchButtonsCount = MIN(count, MAX_BUTTONS);

	for (int i = 0; i < activeTouchButtonsCount; i++) {
		buttonState[i].button = buttons + i;
		buttonState[i].dirty = 1;
		buttonState[i].pressed = 0;
		buttonState[i].counter = 0;
	}
}

void touchbuttonsRender()
{
	//PROFILE_ENTER(drawlist);
	for(int i = 0; i < activeTouchButtonsCount; i++) {
		if (buttonState[i].dirty) {
			const TouchButton* button = buttonState[i].button;
			renderTouchButton(button, buttonState[i].pressed ? BUTTON_FLASH_COLOUR : button->colour);
			buttonState[i].dirty = 0;
		}
	}
	//PROFILE_EXIT(drawlist);
}

int touchButtonsUpdate(const Event** eventTriggered)
{
	int result = EVENT_NONE;

	for(int i = 0; i < activeTouchButtonsCount; i++) {
		if (buttonState[i].counter > 0) {
			buttonState[i].counter--;
			if (buttonState[i].counter == 0) {
				buttonState[i].pressed = 0;
				buttonState[i].dirty = 1;
			}
		}
	}

	touchStateCounter++;

	switch (touchState) {
		case TOUCH_STATE_PENDING: {
			if (touchStateCounter > TOUCH_DEBOUNCE_THRESHOLD) {
				Point touch;

				if (touchScreenGetCoordinates(&touch)) {
					int touchButton = hitTestTouchButtons(&touch);
					if (touchButton == currentTouchButton) {
						touchState = TOUCH_STATE_ACTIVE;
						if (activeTouchButtons[currentTouchButton].flags & TB_PRESS_ACTIVATE) {
							*eventTriggered = activeTouchButtons[currentTouchButton].event;
							result = (*eventTriggered)->type;
						}
					}
					else {
						touchState = TOUCH_STATE_IDLE;
					}
				}
				else {
					touchState = TOUCH_STATE_IDLE;
				}

				if (touchState == TOUCH_STATE_IDLE) {
					setCurrentButtonPressedState(0);
				}
			}
			break;
		}
		case TOUCH_STATE_ACTIVE: {
			Point touch;
			if (!touchScreenGetCoordinates(&touch)) {
				touchState = TOUCH_STATE_IDLE;
				setCurrentButtonPressedState(0);
				if (currentTouchButton >= 0) {
					if (!(activeTouchButtons[currentTouchButton].flags & TB_PRESS_ACTIVATE)) {
						*eventTriggered = activeTouchButtons[currentTouchButton].event;
						result = (*eventTriggered)->type;
					}
				}
			}
			break;
		}
		default: {
			break;
		}
	}

	return result;
}

void touchbuttonsProcessTouch(const Point* touch)
{
	switch (touchState) {
		case TOUCH_STATE_IDLE: {
			touchState = TOUCH_STATE_PENDING;
			touchStateCounter = 0;
			currentTouchButton = hitTestTouchButtons(touch);
			setCurrentButtonPressedState(1);
			break;
		}
		default: {
			break;
		}
	}
}

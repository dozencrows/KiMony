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

#define MAX_BUTTONS				24

#define BUTTON_BORDER_COLOUR	0xffff
#define BUTTON_FLASH_COLOUR		0xffff
#define BUTTON_FLASH_COUNT		15

typedef struct _ButtonState {
	const TouchButton*	button;
	unsigned int dirty:1;
	unsigned int pressed:1;
	unsigned int counter:6;
} ButtonState;

static const TouchButton* activeTouchButtons = NULL;
static int activeTouchButtonsCount = 0;
static ButtonState buttonState[MAX_BUTTONS];

static void renderTouchButton(const TouchButton* button, uint16_t colour)
{
	rendererDrawHLine(button->x, button->y, button->width, BUTTON_BORDER_COLOUR);
	rendererDrawVLine(button->x, button->y, button->height, BUTTON_BORDER_COLOUR);
	rendererDrawVLine(button->x + button->width - 1, button->y, button->height, BUTTON_BORDER_COLOUR);
	rendererDrawRect(button->x + 1, button->y + 1, button->width - 2, button->height - 2, colour);
	rendererDrawHLine(button->x, button->y + button->height - 1, button->width, BUTTON_BORDER_COLOUR);
	rendererDrawString("BTNX", button->x + 3, button->y + 3, &KiMony, 0xffff);
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
	for(int i = 0; i < activeTouchButtonsCount; i++) {
		if (buttonState[i].dirty) {
			const TouchButton* button = buttonState[i].button;
			renderTouchButton(button, buttonState[i].pressed ? BUTTON_FLASH_COLOUR : button->colour);
			buttonState[i].dirty = 0;
		}
	}
}

void touchButtonsUpdate()
{
	for(int i = 0; i < activeTouchButtonsCount; i++) {
		if (buttonState[i].counter > 0) {
			buttonState[i].counter--;
			if (buttonState[i].counter == 0) {
				buttonState[i].pressed = 0;
				buttonState[i].dirty = 1;
			}
		}
	}
}

int touchbuttonsProcessTouch(const Point* touch, const IrAction** action)
{
	int result = TOUCHBUTTON_EVENT_NONE;

	for(int i = 0; i < activeTouchButtonsCount; i++) {
		const TouchButton* button = buttonState[i].button;

		if (touch->x >= button->x && button->width > touch->x - button->x) {
			if (touch->y >= button->y && button->height > touch->y - button->y) {
				result = TOUCHBUTTON_EVENT_IRACTION;
				*action = button->action;
				buttonState[i].pressed = 1;
				buttonState[i].dirty = 1;
				buttonState[i].counter = BUTTON_FLASH_COUNT;
				break;
			}
		}
	}

	return result;
}

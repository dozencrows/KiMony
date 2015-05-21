/*
 * buttons.c
 *
 *  Created on: 21 May 2015
 *      Author: ntuckett
 */
#include <stddef.h>
#include "buttons.h"
#include "keymatrix.h"

static const ButtonMapping* activeMapping = NULL;
static int activeMappingCount = 0;
static uint32_t buttonsState = 0;
static uint32_t buttonsNewState = 0;

void buttonsSetActiveMapping(const ButtonMapping* mapping, int count)
{
	activeMapping = mapping;
	activeMappingCount = count;
}

void buttonsInit()
{
	buttonsPollState();
	buttonsState = buttonsNewState;
}

void buttonsPollState()
{
	buttonsNewState = keyMatrixPoll() | keyNonMatrixPoll();
}

int buttonsUpdate(const IrAction** action)
{
	int result = BUTTON_EVENT_NONE;
	uint32_t buttonChange = buttonsState ^ buttonsNewState;

	if (buttonChange) {
		uint32_t buttonsNewOn = buttonsNewState & buttonChange;

		if (buttonsNewOn) {
			for (size_t i = 0; i < activeMappingCount; i++) {
				if (activeMapping[i].buttonMask == buttonsNewOn) {
					*action = activeMapping[i].action;
					result = BUTTON_EVENT_IRACTION;
					break;
				}
			}
		}

		buttonsState = buttonsNewState;
	}

	return result;
}

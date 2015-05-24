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

int buttonsUpdate(const Event** eventTriggered)
{
	int result = EVENT_NONE;
	uint32_t buttonChange = buttonsState ^ buttonsNewState;

	if (buttonChange) {
		uint32_t buttonsNewOn = buttonsNewState & buttonChange;

		if (buttonsNewOn) {
			for (size_t i = 0; i < activeMappingCount; i++) {
				if (activeMapping[i].buttonMask == buttonsNewOn) {
					*eventTriggered = activeMapping[i].event;
					result = (*eventTriggered)->type;
					break;
				}
			}
		}

		buttonsState = buttonsNewState;
	}

	return result;
}

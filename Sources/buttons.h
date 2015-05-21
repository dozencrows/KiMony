/*
 * buttons.h
 *
 *  Created on: 21 May 2015
 *      Author: ntuckett
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>

typedef struct _IrAction IrAction;

typedef struct _ButtonMapping {
	uint32_t		buttonMask;
	const IrAction*	action;
} ButtonMapping;

#define BUTTON_EVENT_NONE		0
#define BUTTON_EVENT_IRACTION	1

extern void buttonsInit();
extern void buttonsSetActiveMapping(const ButtonMapping* mapping, int count);
extern void buttonsPollState();
extern int buttonsUpdate(const IrAction** action);

#endif /* BUTTONS_H_ */

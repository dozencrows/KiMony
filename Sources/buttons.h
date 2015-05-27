/*
 * buttons.h
 *
 *  Created on: 21 May 2015
 *      Author: ntuckett
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>
#include "event.h"

typedef struct _ButtonMapping {
	uint32_t	buttonMask;
	uint32_t	eventOffset;
} ButtonMapping;

extern void buttonsInit();
extern void buttonsSetActiveMapping(const ButtonMapping* mapping, int count);
extern void buttonsPollState();
extern int buttonsUpdate(const Event** eventTriggered);

#endif /* BUTTONS_H_ */

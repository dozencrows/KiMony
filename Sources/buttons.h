//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

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
extern int buttonsPollState();
extern void buttonsClearState();
extern int buttonsUpdate(const Event** eventTriggered);

#endif /* BUTTONS_H_ */

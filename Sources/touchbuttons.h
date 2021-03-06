//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * touchbuttons.h
 *
 *  Created on: 21 May 2015
 *      Author: ntuckett
 */

#ifndef TOUCHBUTTONS_H_
#define TOUCHBUTTONS_H_

#include <stdint.h>
#include "event.h"

typedef struct _IrAction IrAction;
typedef struct _Point Point;

#define TB_PRESS_ACTIVATE	0x01
#define TB_CENTRE_TEXT		0x02
#define TB_NO_BORDER		0x04
#define TB_NO_FILL			0x08

typedef struct _TouchButton
{
    uint32_t eventOffset;
    uint32_t textOffset;
    uint16_t x, y;
    uint16_t width, height;
    uint16_t colour;
    uint16_t flags;
    uint32_t imageOffsets[2];
} TouchButton;

extern void touchbuttonsInit();
extern void touchbuttonsSetActive(const TouchButton* buttons, int count);
extern void touchbuttonsRender();
extern void touchbuttonsRedraw();
extern void touchbuttonsProcessTouch(const Point* touch);
extern int touchButtonsUpdate(const Event** eventTriggered);

#endif /* TOUCHBUTTONS_H_ */

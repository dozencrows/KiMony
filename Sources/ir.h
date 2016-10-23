//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * ir.h
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#ifndef IR_H_
#define IR_H_
#include <stdint.h>

#define IRCODE_NOP	0
#define IRCODE_RC6	1
#define IRCODE_SIRC	2

#define IR_ACTION_QUEUED	1
#define IR_ACTION_IGNORED	0

typedef struct _IrCode
{
    unsigned int encoding :4;
    unsigned int bits :5;
    unsigned int code :23;
    uint32_t toggleMask;
} IrCode;

typedef struct _IrAction
{
    int codeCount;
    IrCode codes[];
} IrAction;

extern void irInit();
extern void irDoAction(const IrAction* action, uint8_t* toggleFlag);
extern int irQueueAction(const IrAction* action, uint8_t* toggleFlag);

#endif /* IR_H_ */

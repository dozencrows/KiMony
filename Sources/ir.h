/*
 * ir.h
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#ifndef IR_H_
#define IR_H_
#include <stdint.h>

extern void irTest();

extern void irSendRC6Code(uint32_t data, int bitCount);
extern void irSendSIRCCode(uint32_t data, int bitCount);

#endif /* IR_H_ */

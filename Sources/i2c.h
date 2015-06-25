//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * i2c.h
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#ifndef I2C_H_
#define I2C_H_

#include <stddef.h>
#include <stdint.h>

extern void i2cInit();
extern void i2cSendByte(uint8_t address, uint8_t reg, uint8_t data);
extern uint8_t i2cReadByte(uint8_t address, uint8_t reg);
extern void i2cSendBlock(uint8_t address, uint8_t* data, size_t length);

extern void i2cChannelSendByte(int channel, uint8_t address, uint8_t reg, uint8_t data);
extern uint8_t i2cChannelReadByte(int channel, uint8_t address, uint8_t reg);
extern void i2cChannelSendBlock(int channel, uint8_t address, uint8_t* data, size_t length);

#endif /* I2C_H_ */

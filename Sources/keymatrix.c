//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * keymatrix.c
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */
#include "keymatrix.h"
#include <stdio.h>

#include "i2c.h"
#include "systick.h"
#include "ports.h"
#include "interrupts.h"

#define MCP_PORTA			0
#define MCP_PORTB			1
#define MCP_REG_IODIR      	0x00
#define MCP_REG_IPOL       	0x02
#define MCP_REG_GPINTEN    	0x04
#define MCP_REG_DEFVAL     	0x06
#define MCP_REG_INTCON     	0x08
#define MCP_REG_IOCON      	0x0a
#define MCP_REG_GPPU       	0x0c
#define MCP_REG_INTF       	0x0e
#define MCP_REG_INTCAP     	0x10
#define MCP_REG_GPIO       	0x12
#define MCP_REG_OLAT       	0x14

#define MCP_REG(port, reg) (port + reg)

#define MCP_REG_PORTA_PIN	12

#define MCP_REG_INT_MASK	(1<<MCP_REG_PORTA_PIN)

#define MCP_I2C_ADDR  		0x20

#define MCP_GPIO_SETUP_MASK	0xf0	// bits 0-3 output, bits 4-7 input
#define MCP_GPIO_POLL_COL1  0x0e
#define MCP_GPIO_POLL_COL2  0x0d
#define MCP_GPIO_POLL_COL3  0x0b
#define MCP_GPIO_POLL_COL4  0x07

static const PortConfig portAPins =
{
	PORTA_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK | PORT_PCR_IRQC_MASK),
	PORT_PCR_MUX(1) | PORT_PCR_IRQC(0x0a),
	1,
	{ MCP_REG_PORTA_PIN }
};

static volatile uint8_t keyMatrixIntFlag = 0;

static void irqHandlerPortA(uint32_t portAISFR)
{
	if (portAISFR & MCP_REG_INT_MASK) {
		keyMatrixIntFlag++;
	}
}

void keyMatrixInit()
{
	// PTA12 is interrupt from MCP_REG

	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

	portInitialise(&portAPins);

	FGPIOA_PDDR &= ~(MCP_REG_INT_MASK);

	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_IODIR), MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPPU), MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_IPOL), MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPIO), 0);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPINTEN), MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_IOCON), 1 << 6);	// OR interrupt pins

	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_IODIR), MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_GPPU), MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_IPOL), MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_GPIO), 0);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_GPINTEN), MCP_GPIO_SETUP_MASK);

	interruptRegisterPortAIRQHandler(irqHandlerPortA);
	NVIC_EnableIRQ(PORTA_IRQn);
}

void keyMatrixClearInterrupt()
{
	i2cReadByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_GPIO));
	i2cReadByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPIO));
	PORTA_ISFR = MCP_REG_INT_MASK;
	keyMatrixIntFlag = 0;
}

int keyMatrixCheckInterrupt()
{
	return keyMatrixIntFlag;
}

uint32_t keyMatrixPoll()
{
	uint32_t key_data;

	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_GPINTEN), 0);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPINTEN), 0);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPIO), MCP_GPIO_POLL_COL1);
	key_data = i2cReadByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPIO)) >> 4;

	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPIO), MCP_GPIO_POLL_COL2);
	key_data = key_data | (i2cReadByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPIO)) & 0xf0);

	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPIO), MCP_GPIO_POLL_COL3);
	key_data = key_data | (i2cReadByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPIO)) & 0xf0) << 4;

	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPIO), MCP_GPIO_POLL_COL4);
	key_data = key_data | (i2cReadByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPIO)) & 0xf0) << 8;

	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_GPIO), MCP_GPIO_POLL_COL1);
	key_data = key_data | (i2cReadByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_GPIO)) & 0xf0) << 12;

	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_GPIO), MCP_GPIO_POLL_COL2);
	key_data = key_data | (i2cReadByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_GPIO)) & 0xf0) << 16;

	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_GPIO), 0);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPIO), 0);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTA, MCP_REG_GPINTEN), MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP_REG(MCP_PORTB, MCP_REG_GPINTEN), MCP_GPIO_SETUP_MASK);
    return key_data;
}

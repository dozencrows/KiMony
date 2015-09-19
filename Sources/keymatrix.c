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

#define MCP23008_IODIR      0x00
#define MCP23008_IPOL       0x01
#define MCP23008_GPINTEN    0x02
#define MCP23008_DEFVAL     0x03
#define MCP23008_INTCON     0x04
#define MCP23008_IOCON      0x05
#define MCP23008_GPPU       0x06
#define MCP23008_INTF       0x07
#define MCP23008_INTCAP     0x08
#define MCP23008_GPIO       0x09
#define MCP23008_OLAT       0x0A

#define MCP23008_PORTE_PIN	20
#define MCP23008_PORTA_PIN	12

#define MCP23008_RESET_MASK	(1<<MCP23008_PORTE_PIN)
#define MCP23008_INT_MASK	(1<<MCP23008_PORTA_PIN)

#define MCP_I2C_ADDR  		0x20

#define MCP_GPIO_SETUP_MASK	0x0f	// bits 0-3 input, bits 4-7 output
#define MCP_GPIO_POLL_COL1  0xe0
#define MCP_GPIO_POLL_COL2  0xd0
#define MCP_GPIO_POLL_COL3  0xb0
#define MCP_GPIO_POLL_COL4  0x70

#define PORTA_KEY0			4
#define PORTB_KEY0			0
#define PORTE_KEY0			21
#define PORTE_KEY1			22
#define PORTE_KEY2			23
#define PORTE_KEY3			29

#define PORTA_KEYBITS		((1<<PORTA_KEY0))
#define PORTB_KEYBITS		((1<<PORTB_KEY0))
#define PORTE_KEYBITS		((1<<PORTE_KEY0)|(1<<PORTE_KEY1)|(1<<PORTE_KEY2)|(1<<PORTE_KEY3))

static const PortConfig portAButtons1 =
{
	PORTA_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK,
	1,
	{ PORTA_KEY0 }
};

static const PortConfig portAButtons2 =
{
	PORTA_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK | PORT_PCR_IRQC_MASK),
	PORT_PCR_MUX(1) | PORT_PCR_IRQC(0x0a),
	1,
	{ MCP23008_PORTA_PIN }
};

static const PortConfig portBButtons =
{
	PORTB_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK,
	1,
	{ PORTB_KEY0 }
};

static const PortConfig portEButtons =
{
	PORTE_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK,
	4,
	{ PORTE_KEY0, PORTE_KEY1, PORTE_KEY2, PORTE_KEY3 }
};

static volatile uint8_t keyMatrixIntFlag = 0;

static void irqHandlerPortA(uint32_t portAISFR)
{
	if (portAISFR & MCP23008_INT_MASK) {
		keyMatrixIntFlag++;
	}
}

void keyMatrixInit()
{
	// PTE20 is reset for MCP23008
	// PTA12 is interrupt from MCP23008

	// Ancilliary buttons on:
	//	PTE21, PTE22, PTE23, PTE30, PTA4, PTB0


	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK|SIM_SCGC5_PORTB_MASK|SIM_SCGC5_PORTE_MASK;

	portInitialise(&portAButtons1);
	portInitialise(&portAButtons2);
	portInitialise(&portBButtons);
	portInitialise(&portEButtons);

	FGPIOA_PDDR &= ~(PORTA_KEYBITS|MCP23008_INT_MASK);
	FGPIOB_PDDR &= ~(PORTB_KEYBITS);
	FGPIOE_PDDR &= ~(PORTE_KEYBITS);

	// PTE20: MCP23008 reset
	PORTE_PCR20 = (uint32_t)((PORTE_PCR20 & (uint32_t)~(uint32_t)(
				 PORT_PCR_ISF_MASK |
				 PORT_PCR_MUX(0x06)
				)) | (uint32_t)(
				 PORT_PCR_MUX(0x01)
				));
	FGPIOE_PDDR |= MCP23008_RESET_MASK;
	FGPIOE_PSOR = MCP23008_RESET_MASK;

	FGPIOE_PCOR = MCP23008_RESET_MASK;	// Reset MCP23008
	FGPIOE_PSOR = MCP23008_RESET_MASK;

	i2cSendByte(MCP_I2C_ADDR, MCP23008_IODIR, MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPPU, MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP23008_IPOL, MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPIO, 0);
	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPINTEN, MCP_GPIO_SETUP_MASK);

	interruptRegisterPortAIRQHandler(irqHandlerPortA);
	NVIC_EnableIRQ(PORTA_IRQn);
}

void keyMatrixClearInterrupt()
{
	i2cReadByte(MCP_I2C_ADDR, MCP23008_GPIO);
	PORTA_ISFR = MCP23008_INT_MASK;
	keyMatrixIntFlag = 0;
}

int keyMatrixCheckInterrupt()
{
	return keyMatrixIntFlag;
}

uint32_t keyMatrixPoll()
{
	uint32_t key_data;

	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPINTEN, 0);
	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPIO, MCP_GPIO_POLL_COL1);
	key_data = i2cReadByte(MCP_I2C_ADDR, MCP23008_GPIO) & 0x0f;

	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPIO, MCP_GPIO_POLL_COL2);
	key_data = key_data | (i2cReadByte(MCP_I2C_ADDR, MCP23008_GPIO) & 0x0f) << 4;

	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPIO, MCP_GPIO_POLL_COL3);
	key_data = key_data | (i2cReadByte(MCP_I2C_ADDR, MCP23008_GPIO) & 0x0f) << 8;

	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPIO, MCP_GPIO_POLL_COL4);
	key_data = key_data | (i2cReadByte(MCP_I2C_ADDR, MCP23008_GPIO) & 0x0f) << 12;

	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPIO, 0);
	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPINTEN, MCP_GPIO_SETUP_MASK);
    return key_data;
}

uint32_t keyNonMatrixPoll()
{
	uint32_t key_data;

	uint32_t portAKeyState = FGPIOA_PDIR;
	uint32_t portBKeyState = FGPIOB_PDIR;
	uint32_t portEKeyState = FGPIOE_PDIR;

	key_data  = ((~portAKeyState & PORTA_KEYBITS) << 12);
	key_data |= ((~portBKeyState & PORTB_KEYBITS) << 17);
	key_data |= ((~portEKeyState & ((1<<PORTE_KEY0)|(1<<PORTE_KEY1)|(1<<PORTE_KEY2))) >> 3);
	key_data |= ((~portEKeyState & (1<<PORTE_KEY3)) >> 8);

    return key_data;
}

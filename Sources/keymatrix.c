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
#include "port_util.h"

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

#define MCP23008_RESET_MASK			(1<<20)
#define MCP23008_INT_MASK			(1<<0)

#define MCP_I2C_ADDR  		0x20

#define MCP_GPIO_SETUP_MASK	0x0f	// bits 0-3 input, bits 4-7 output
#define MCP_GPIO_POLL_COL1  0xe0
#define MCP_GPIO_POLL_COL2  0xd0
#define MCP_GPIO_POLL_COL3  0xb0
#define MCP_GPIO_POLL_COL4  0x70

static const PortConfig portAButtons =
{
	PORTA_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK,
	2,
	{ 4, 12 }
};

static const PortConfig portEButtons =
{
	PORTE_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK,
	4,
	{ 21, 22, 23, 30 }
};

void keyMatrixInit()
{
	// PTE20 is reset for MCP23008
	// PTB0 is interrupt from MCP23008

	// Ancilliary buttons on:
	//	PTE21, PTE22, PTE23, PTE30, PTA4, PTA12


	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK|SIM_SCGC5_PORTB_MASK|SIM_SCGC5_PORTE_MASK;

	portInitialise(&portAButtons);
	portInitialise(&portEButtons);

	FGPIOA_PDDR &= ~((1<<4)|(1<<12));
	FGPIOE_PDDR &= ~((1<<21)|(1<<22)|(1<<23)|(1<<30));

	// PTE20: MCP23008 reset
	PORTE_PCR20 = (uint32_t)((PORTE_PCR20 & (uint32_t)~(uint32_t)(
				 PORT_PCR_ISF_MASK |
				 PORT_PCR_MUX(0x06)
				)) | (uint32_t)(
				 PORT_PCR_MUX(0x01)
				));
	FGPIOE_PDDR |= MCP23008_RESET_MASK;
	FGPIOE_PSOR = MCP23008_RESET_MASK;

	// PTB0: MCP23008 reset
	PORTB_PCR0 = (uint32_t)((PORTB_PCR0 & (uint32_t)~(uint32_t)(
				 PORT_PCR_ISF_MASK |
				 PORT_PCR_MUX(0x06)
				)) | (uint32_t)(
				 PORT_PCR_MUX(0x01)
				));
	FGPIOB_PDDR &= ~MCP23008_INT_MASK;

	FGPIOE_PCOR = MCP23008_RESET_MASK;	// Reset MCP23008
	FGPIOE_PSOR = MCP23008_RESET_MASK;

	i2cSendByte(MCP_I2C_ADDR, MCP23008_IODIR, MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPPU, MCP_GPIO_SETUP_MASK);
	i2cSendByte(MCP_I2C_ADDR, MCP23008_IPOL, MCP_GPIO_SETUP_MASK);
}

uint32_t keyMatrixPoll()
{
	uint32_t key_data;

	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPIO, MCP_GPIO_POLL_COL1);
	key_data = i2cReadByte(MCP_I2C_ADDR, MCP23008_GPIO) & 0x0f;

	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPIO, MCP_GPIO_POLL_COL2);
	key_data = key_data | (i2cReadByte(MCP_I2C_ADDR, MCP23008_GPIO) & 0x0f) << 4;

	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPIO, MCP_GPIO_POLL_COL3);
	key_data = key_data | (i2cReadByte(MCP_I2C_ADDR, MCP23008_GPIO) & 0x0f) << 8;

	i2cSendByte(MCP_I2C_ADDR, MCP23008_GPIO, MCP_GPIO_POLL_COL4);
	key_data = key_data | (i2cReadByte(MCP_I2C_ADDR, MCP23008_GPIO) & 0x0f) << 12;

	uint32_t portAKeyState = FGPIOA_PDIR & ((1<<4)|(1<<12));
	key_data |= ((~portAKeyState & (1<<4)) << 12);
	key_data |= ((~portAKeyState & (1<<12)) << 5);

	uint32_t portEKeyState = FGPIOE_PDIR & ((1<<21)|(1<<22)|(1<<23)|(1<<30));
	key_data |= ((~portEKeyState & ((1<<21)|(1<<22)|(1<<23))) >> 3);
	key_data |= ((~portEKeyState & (1<<30)) >> 9);

    return key_data;
}

void testKeyMatrix()
{
	uint32_t keypadLast = keyMatrixPoll();

	while (keypadLast != 0x1111) {
		uint32_t keypadVal = keyMatrixPoll();
		if (keypadVal != keypadLast) {
			keypadLast = keypadVal;
			printf("Keypad: %08x\n", keypadVal);
		}

		systickDelayMs(50);
	}
}

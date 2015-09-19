//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * i2c.c
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */
#include "i2c.h"

#include "MKL26Z4.h"
#include "ports.h"

// Pins to initialise
// PTE: 0 (SCL)
//		1 (SDA)
//		24 (SCL)
//		25 (SDA)

static I2C_Type * const i2cChannel[2] = I2C_BASE_PTRS;

static const PortConfig channel0PortEPins =
{
	PORTE_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(5),
	2,
	{ 24, 25 }
};

static const PortConfig channel1PortEPins =
{
	PORTE_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(6),
	2,
	{ 0, 1 }
};

static void i2cInitChannel(I2C_Type * channel)
{
	/* I2C1_FLT: SHEN=0,STOPF=0,STOPIE=0,FLT=0 */
	I2C_FLT_REG(channel) = I2C_FLT_FLT(0x00);
	/* I2C1_A1: AD=0,??=0 */
	I2C_A1_REG(channel) = I2C_A1_AD(0x00);
	/* I2C1_C2: GCAEN=0,ADEXT=0,HDRS=0,SBRC=0,RMEN=0,AD=0 */
	I2C_C2_REG(channel) = I2C_C2_AD(0x00);
	/* I2C1_RA: RAD=0,??=0 */
	I2C_RA_REG(channel) = I2C_RA_RAD(0x00);
	/* I2C1_F: MULT=0,ICR=0 */
	I2C_F_REG(channel) = (I2C_F_MULT(0x00) | I2C_F_ICR(0x00));
	/* I2C1_A2: SAD=0x61,??=0 */
	I2C_A2_REG(channel) = I2C_A2_SAD(0x61);
	/* I2C1_SMB: FACK=0,ALERTEN=0,SIICAEN=0,TCKSEL=0,SLTF=1,SHTF1=0,SHTF2=1,SHTF2IE=0 */
	I2C_SMB_REG(channel) = (I2C_SMB_SLTF_MASK | I2C_SMB_SHTF2_MASK);
	/* I2C1_SLTH: SSLT=0 */
	I2C_SLTH_REG(channel) = I2C_SLTH_SSLT(0x00);
	/* I2C1_SLTL: SSLT=0 */
	I2C_SLTL_REG(channel) = I2C_SLTL_SSLT(0x00);
	/* I2C1_S: TCF=0,IAAS=0,BUSY=0,ARBL=1,RAM=0,SRW=0,IICIF=1,RXAK=0 */
	I2C_S_REG(channel) = (I2C_S_ARBL_MASK | I2C_S_IICIF_MASK);
	/* I2C1_C1: IICEN=0,IICIE=0,MST=0,TX=0,TXAK=0,RSTA=0,WUEN=0,DMAEN=0 */
	I2C_C1_REG(channel) = 0x00U;

	I2C_F_REG(channel)   = (I2C_F_MULT(0x00) | I2C_F_ICR(0x27));		// Mult = 1, Divider = 480 -> 100KHz for 48MHz system clock

	I2C_C1_REG(channel) |= I2C_C1_IICEN_MASK;							// Enable module
}

static void i2cSendByteToChannel(I2C_Type * channel, uint8_t address, uint8_t reg, uint8_t data)
{
	while ((I2C_S_REG(channel) & I2C_S_BUSY_MASK));

	I2C_C1_REG(channel) |= I2C_C1_TX_MASK;					// Set up transmit
	I2C_C1_REG(channel) |= I2C_C1_MST_MASK;					// Generate START
	I2C_D_REG(channel)	= address << 1;						// Send address with R/W bit 0 (write)
	while (!(I2C_S_REG(channel) & I2C_S_BUSY_MASK));
	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;

	I2C_D_REG(channel) = reg;
	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK)) {
		__asm("nop");
	}
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;

	I2C_D_REG(channel) = data;
	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK)) {
		__asm("nop");
	}
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;

	I2C_C1_REG(channel) &= ~(I2C_C1_MST_MASK|I2C_C1_TX_MASK);
}

static uint8_t i2cReadByteFromChannel(I2C_Type * channel, uint8_t address, uint8_t reg)
{
	while ((I2C_S_REG(channel) & I2C_S_BUSY_MASK));

	I2C_C1_REG(channel) |= I2C_C1_TX_MASK;					// Set up transmit
	I2C_C1_REG(channel) |= I2C_C1_MST_MASK;					// Generate START
	I2C_D_REG(channel)	= address << 1;						// Send address with R/W bit 0 (write)
	while (!(I2C_S_REG(channel) & I2C_S_BUSY_MASK));
	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;

	I2C_D_REG(channel) = reg;
	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;

	I2C_C1_REG(channel) |= I2C_C1_RSTA_MASK;
	I2C_D_REG(channel)	= address << 1 | 1;					// Send address with R/W bit 1 (read)
	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;

	I2C_C1_REG(channel) &= ~I2C_C1_TX_MASK;					// Switch to receive
	I2C_C1_REG(channel) |= I2C_C1_TXAK_MASK;
	uint8_t dummy_read = I2C_D_REG(channel);				// Trigger read of next byte

	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;
	I2C_C1_REG(channel) &= ~I2C_C1_MST_MASK;
	uint8_t	data = I2C_D_REG(channel);
	I2C_C1_REG(channel) &= ~I2C_C1_TXAK_MASK;
	return data;
}

void i2cSendBlockToChannel(I2C_Type * channel, uint8_t address, uint8_t* data, size_t length)
{
	while ((I2C_S_REG(channel) & I2C_S_BUSY_MASK));

	I2C_C1_REG(channel) |= I2C_C1_TX_MASK;					// Set up transmit
	I2C_C1_REG(channel) |= I2C_C1_MST_MASK;					// Generate START
	I2C_D_REG(channel)	= address << 1;						// Send address with R/W bit 0 (write)
	while (!(I2C_S_REG(channel) & I2C_S_BUSY_MASK));
	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;

	while (length > 0) {
		I2C_D_REG(channel) = *data++;
		length--;
		while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK)) {
			__asm("nop");
		}
		I2C_S_REG(channel) |= I2C_S_IICIF_MASK;
	}

	I2C_C1_REG(channel) &= ~(I2C_C1_MST_MASK|I2C_C1_TX_MASK);	// generate STOP
}

void i2cReadBlockFromChannel(I2C_Type * channel, uint8_t address, uint8_t reg, uint8_t* data, size_t length)
{
	while ((I2C_S_REG(channel) & I2C_S_BUSY_MASK));

	I2C_C1_REG(channel) |= I2C_C1_TX_MASK;					// Set up transmit
	I2C_C1_REG(channel) |= I2C_C1_MST_MASK;					// Generate START
	I2C_D_REG(channel)	= address << 1;						// Send address with R/W bit 0 (write)
	while (!(I2C_S_REG(channel) & I2C_S_BUSY_MASK));
	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;

	I2C_D_REG(channel) = reg;
	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;

	I2C_C1_REG(channel) |= I2C_C1_RSTA_MASK;
	I2C_D_REG(channel)	= address << 1 | 1;					// Send address with R/W bit 1 (read)
	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;

	I2C_C1_REG(channel) &= ~I2C_C1_TX_MASK;					// Switch to receive
	uint8_t dummy_read = I2C_D_REG(channel);				// Trigger read of next byte

	while (length > 2) {
		while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
		I2C_S_REG(channel) |= I2C_S_IICIF_MASK;
		*data++ = I2C_D_REG(channel);
		length--;
	}

	I2C_C1_REG(channel) |= I2C_C1_TXAK_MASK;
	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;
	*data++ = I2C_D_REG(channel);

	while (!(I2C_S_REG(channel) & I2C_S_IICIF_MASK));
	I2C_S_REG(channel) |= I2C_S_IICIF_MASK;
	I2C_C1_REG(channel) &= ~I2C_C1_MST_MASK;
	*data++ = I2C_D_REG(channel);
	I2C_C1_REG(channel) &= ~I2C_C1_TXAK_MASK;
}

void i2cInit()
{
	SIM_SCGC4 |= SIM_SCGC4_I2C0_MASK|SIM_SCGC4_I2C1_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	//portInitialise(&channel0PortEPins);
	portInitialise(&channel1PortEPins);

	//i2cInitChannel(I2C0);
	i2cInitChannel(I2C1);
}

void i2cSendByte(uint8_t address, uint8_t reg, uint8_t data)
{
	i2cSendByteToChannel(I2C1, address, reg, data);
}

uint8_t i2cReadByte(uint8_t address, uint8_t reg)
{
	return i2cReadByteFromChannel(I2C1, address, reg);
}

void i2cSendBlock(uint8_t address, uint8_t* data, size_t length)
{
	i2cSendBlockToChannel(I2C1, address, data, length);
}

void i2cChannelSendByte(int channel, uint8_t address, uint8_t reg, uint8_t data)
{
	i2cSendByteToChannel(i2cChannel[channel], address, reg, data);
}

uint8_t i2cChannelReadByte(int channel, uint8_t address, uint8_t reg)
{
	return i2cReadByteFromChannel(i2cChannel[channel], address, reg);
}

void i2cChannelSendBlock(int channel, uint8_t address, uint8_t* data, size_t length)
{
	i2cSendBlockToChannel(i2cChannel[channel], address, data, length);
}

void i2cChannelReadBlock(int channel, uint8_t address, uint8_t reg, uint8_t* data, size_t length)
{
	i2cReadBlockFromChannel(i2cChannel[channel], address, reg, data, length);
}

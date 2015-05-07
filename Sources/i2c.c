/*
 * i2c.c
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */
#include "i2c.h"

#include "MKL26Z4.h"
#include "port_util.h"

// Pins to initialise
// PTE: 0 (SCLK)
//		1 (MOSI)

static const PortConfig portEPins =
{
	PORTE_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(6),
	2,
	{ 0, 1 }
};

void i2cInit()
{
	/* SIM_SCGC4: I2C1=1 */
	SIM_SCGC4 |= SIM_SCGC4_I2C1_MASK;
	/* I2C1_FLT: SHEN=0,STOPF=0,STOPIE=0,FLT=0 */
	I2C1_FLT = I2C_FLT_FLT(0x00);
	/* I2C1_A1: AD=0,??=0 */
	I2C1_A1 = I2C_A1_AD(0x00);
	/* I2C1_C2: GCAEN=0,ADEXT=0,HDRS=0,SBRC=0,RMEN=0,AD=0 */
	I2C1_C2 = I2C_C2_AD(0x00);
	/* I2C1_RA: RAD=0,??=0 */
	I2C1_RA = I2C_RA_RAD(0x00);
	/* I2C1_F: MULT=0,ICR=0 */
	I2C1_F = (I2C_F_MULT(0x00) | I2C_F_ICR(0x00));
	/* I2C1_A2: SAD=0x61,??=0 */
	I2C1_A2 = I2C_A2_SAD(0x61);
	/* I2C1_SMB: FACK=0,ALERTEN=0,SIICAEN=0,TCKSEL=0,SLTF=1,SHTF1=0,SHTF2=1,SHTF2IE=0 */
	I2C1_SMB = (I2C_SMB_SLTF_MASK | I2C_SMB_SHTF2_MASK);
	/* I2C1_SLTH: SSLT=0 */
	I2C1_SLTH = I2C_SLTH_SSLT(0x00);
	/* I2C1_SLTL: SSLT=0 */
	I2C1_SLTL = I2C_SLTL_SSLT(0x00);
	/* I2C1_S: TCF=0,IAAS=0,BUSY=0,ARBL=1,RAM=0,SRW=0,IICIF=1,RXAK=0 */
	I2C1_S = (I2C_S_ARBL_MASK | I2C_S_IICIF_MASK);
	/* I2C1_C1: IICEN=0,IICIE=0,MST=0,TX=0,TXAK=0,RSTA=0,WUEN=0,DMAEN=0 */
	I2C1_C1 = 0x00U;

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	portInitialise(&portEPins);

	I2C1_F   = (I2C_F_MULT(0x00) | I2C_F_ICR(0x27));		// Mult = 1, Divider = 480 -> 100KHz for 48MHz clock
	I2C1_C1 |= I2C_C1_IICEN_MASK;							// Enable module
}

void i2cSendByte(uint8_t address, uint8_t reg, uint8_t data)
{
	while ((I2C1_S & I2C_S_BUSY_MASK));

	I2C1_C1 |= I2C_C1_TX_MASK;					// Set up transmit
	I2C1_C1 |= I2C_C1_MST_MASK;					// Generate START
	I2C1_D	= address << 1;						// Send address with R/W bit 0 (write)
	while (!(I2C1_S & I2C_S_BUSY_MASK));
	while (!(I2C1_S & I2C_S_IICIF_MASK));
	I2C1_S |= I2C_S_IICIF_MASK;

	I2C1_D = reg;
	while (!(I2C1_S & I2C_S_IICIF_MASK)) {
		__asm("nop");
	}
	I2C1_S |= I2C_S_IICIF_MASK;

	I2C1_D = data;
	while (!(I2C1_S & I2C_S_IICIF_MASK)) {
		__asm("nop");
	}
	I2C1_S |= I2C_S_IICIF_MASK;

	I2C1_C1 &= ~(I2C_C1_MST_MASK|I2C_C1_TX_MASK);
}

uint8_t i2cReadByte(uint8_t address, uint8_t reg)
{
	while ((I2C1_S & I2C_S_BUSY_MASK));

	I2C1_C1 |= I2C_C1_TX_MASK;					// Set up transmit
	I2C1_C1 |= I2C_C1_MST_MASK;					// Generate START
	I2C1_D	= address << 1;						// Send address with R/W bit 0 (write)
	while (!(I2C1_S & I2C_S_BUSY_MASK));
	while (!(I2C1_S & I2C_S_IICIF_MASK));
	I2C1_S |= I2C_S_IICIF_MASK;

	I2C1_D = reg;
	while (!(I2C1_S & I2C_S_IICIF_MASK));
	I2C1_S |= I2C_S_IICIF_MASK;

	I2C1_C1 |= I2C_C1_RSTA_MASK;
	I2C1_D	= address << 1 | 1;					// Send address with R/W bit 1 (read)
	while (!(I2C1_S & I2C_S_IICIF_MASK));
	I2C1_S |= I2C_S_IICIF_MASK;

	I2C1_C1 &= ~I2C_C1_TX_MASK;					// Switch to receive
	uint8_t dummy_read = I2C1_D;				// Trigger read of next byte

	while (!(I2C1_S & I2C_S_IICIF_MASK));
	I2C1_S |= I2C_S_IICIF_MASK;
	uint8_t	data = I2C1_D;
	I2C1_C1 &= ~I2C_C1_MST_MASK;
	return data;
}

void i2cSendBlock(uint8_t address, uint8_t* data, size_t length)
{
	while ((I2C1_S & I2C_S_BUSY_MASK));

	I2C1_C1 |= I2C_C1_TX_MASK;					// Set up transmit
	I2C1_C1 |= I2C_C1_MST_MASK;					// Generate START
	I2C1_D	= address << 1;						// Send address with R/W bit 0 (write)
	while (!(I2C1_S & I2C_S_BUSY_MASK));
	while (!(I2C1_S & I2C_S_IICIF_MASK));
	I2C1_S |= I2C_S_IICIF_MASK;

	while (length > 0) {
		I2C1_D = *data++;
		length--;
		while (!(I2C1_S & I2C_S_IICIF_MASK)) {
			__asm("nop");
		}
		I2C1_S |= I2C_S_IICIF_MASK;
	}

	I2C1_C1 &= ~(I2C_C1_MST_MASK|I2C_C1_TX_MASK);	// generate STOP
}






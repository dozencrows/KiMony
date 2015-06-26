/*
 * accelerometer.c
 *
 *  Created on: 25 Jun 2015
 *      Author: ntuckett
 */
#include "accelerometer.h"
#include <MKL26Z4.h>
#include <stdint.h>
#include <stdio.h>
#include "i2c.h"
#include "ports.h"
#include "interrupts.h"

#define FXOS8700CQ_I2C_CHANNEL		0
#define FXOS8700CQ_SLAVE_ADDR		0x1d

#define FXOS8700CQ_STATUS			0x00
#define FXOS8700CQ_INT_SOURCE		0x0C
#define FXOS8700CQ_WHOAMI			0x0D
#define FXOS8700CQ_XYZ_DATA_CFG		0x0E
#define FXOS8700CQ_TRANSIENT_CFG	0x1D
#define FXOS8700CQ_TRANSIENT_SRC	0x1E
#define FXOS8700CQ_TRANSIENT_THS	0x1F
#define FXOS8700CQ_TRANSIENT_COUNT	0x20

#define FXOS8700CQ_CTRL_REG1		0x2A
#define FXOS8700CQ_CTRL_REG2		0x2B
#define FXOS8700CQ_CTRL_REG3		0x2C
#define FXOS8700CQ_CTRL_REG4		0x2D
#define FXOS8700CQ_CTRL_REG5		0x2E

#define FXOS8700CQ_M_CTRL_REG1		0x5B
#define FXOS8700CQ_M_CTRL_REG2		0x5C

#define FXOS8700CQ_WHOAMI_VAL	0xC7
#define FXOS8700CQ_PACKET_SIZE	13

#define FXOS8700CQ_INT_TRANS_MASK	0x20



typedef struct _SensorData {
	int16_t	x, y, z;
} SensorData;

static int accelInitialised = 0;

static const PortConfig portDPins =
{
	PORTD_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK | PORT_PCR_IRQC_MASK),
	PORT_PCR_MUX(1) | PORT_PCR_IRQC(0x0a),
	2,
	{ 0, 1 }
};

static volatile int accelerometerInt1Flag = 0;

static void irqHandlerPortCD(uint32_t portCISFR, uint32_t portDISFR)
{
	if (portDISFR & 1) {
		accelerometerInt1Flag++;
	}
}

void accelClearInterrupts()
{
	uint8_t intSource = i2cChannelReadByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_INT_SOURCE);

	if (intSource & FXOS8700CQ_INT_TRANS_MASK) {
		volatile uint8_t transientStatus = i2cChannelReadByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_TRANSIENT_SRC);
		accelerometerInt1Flag = 0;
	}
}

void accelInit()
{
	uint8_t whoAmI = i2cChannelReadByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_WHOAMI);

	if (whoAmI == FXOS8700CQ_WHOAMI_VAL) {
		SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
		portInitialise(&portDPins);

		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, 0x18);			// place in standby, 100Hz data rate
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_TRANSIENT_CFG, 0x1e);		// Latch, X, Y and Z enabled
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_TRANSIENT_THS, 0x03);		// 0.18g threshold
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_TRANSIENT_COUNT, 0x05);	// Debounce to 50ms
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG4, FXOS8700CQ_INT_TRANS_MASK);			// Enable interrupt
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG5, FXOS8700CQ_INT_TRANS_MASK);			// Route interrupt

		interruptRegisterPortCDIRQHandler(irqHandlerPortCD);
		NVIC_EnableIRQ(PORTC_PORTD_IRQn);

		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, 0x19);			// activate with 100Hz data rate

		accelInitialised = 1;
	}
}

int accelCheckTransientInterrupt()
{
	return accelerometerInt1Flag > 0;
}

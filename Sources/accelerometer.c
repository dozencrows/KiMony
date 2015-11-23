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
#include "systick.h"
#include "ports.h"
#include "interrupts.h"
#include "debugutils.h"

#define FXOS8700CQ_I2C_CHANNEL		1
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

#define FXOS8700CQ_CTRL_REG1_ACTIVE_BIT		0
#define FXOS8700CQ_CTRL_REG1_FREAD_BIT		1
#define FXOS8700CQ_CTRL_REG1_LNOISE_BIT		2
#define FXOS8700CQ_CTRL_REG1_ODR_BIT		3
#define FXOS8700CQ_CTRL_REG1_ASLP_BIT		6

#define FXOS8700CQ_CTRL_REG1_ACTIVE_MASK	(1 << FXOS8700CQ_CTRL_REG1_ACTIVE_BIT)
#define FXOS8700CQ_CTRL_REG1_FREAD_MASK		(1 << FXOS8700CQ_CTRL_REG1_FREAD_BIT)
#define FXOS8700CQ_CTRL_REG1_LNOISE_MASK	(1 << FXOS8700CQ_CTRL_REG1_LNOISE_BIT)
#define FXOS8700CQ_CTRL_REG1_ODR_MASK		(7 << FXOS8700CQ_CTRL_REG1_ODR_BIT)
#define FXOS8700CQ_CTRL_REG1_ODR(x)			((x & 7) << FXOS8700CQ_CTRL_REG1_ODR_BIT)
#define FXOS8700CQ_CTRL_REG1_ASLP_MASK		(3 << FXOS8700CQ_CTRL_REG1_ASLP_BIT)
#define FXOS8700CQ_CTRL_REG1_ASLP(x)		((x & 3) << FXOS8700CQ_CTRL_REG1_ASLP_BIT)

#define FXOS8700CQ_INT_TRANS_MASK		0x20

#define FXOS8700CQ_TRANS_CFG_HPFBYP_BIT		0
#define FXOS8700CQ_TRANS_CFG_XEFE_BIT		1
#define FXOS8700CQ_TRANS_CFG_YEFE_BIT		2
#define FXOS8700CQ_TRANS_CFG_ZEFE_BIT		3
#define FXOS8700CQ_TRANS_CFG_ELE_BIT		4

#define FXOS8700CQ_TRANS_CFG_HPFBYP_MASK	(1 << FXOS8700CQ_TRANS_CFG_HPFBYP_BIT)
#define FXOS8700CQ_TRANS_CFG_XEFE_MASK		(1 << FXOS8700CQ_TRANS_CFG_XEFE_BIT)
#define FXOS8700CQ_TRANS_CFG_YEFE_MASK		(1 << FXOS8700CQ_TRANS_CFG_YEFE_BIT)
#define FXOS8700CQ_TRANS_CFG_ZEFE_MASK		(1 << FXOS8700CQ_TRANS_CFG_ZEFE_BIT)
#define FXOS8700CQ_TRANS_CFG_ELE_MASK		(1 << FXOS8700CQ_TRANS_CFG_ELE_BIT)

#define FXOS8700CQ_TRANS_THS_THS_BIT		0
#define FXOS8700CQ_TRANS_THS_DBCNTM_BIT		7

#define FXOS8700CQ_TRANS_THS_THS_MASK		(0x7f << FXOS8700CQ_TRANS_THS_THS_BIT)
#define FXOS8700CQ_TRANS_THS_THS(x)			((x & 0x7f) << FXOS8700CQ_TRANS_THS_THS_BIT)
#define FXOS8700CQ_TRANS_THS_DBCNTM_MASK	(1 << FXOS8700CQ_TRANS_THS_DBCNTM_BIT)

typedef struct _SensorData {
	int16_t	x, y, z;
} SensorData;

static int accelInitialised = 0;

static const PortConfig portDPins =
{
	PORTD_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK | PORT_PCR_IRQC_MASK),
	PORT_PCR_MUX(1) | PORT_PCR_IRQC(0x0a) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK,
	2,
	{ 0, 1 }
};

static volatile int accelerometerInt1Flag = 0;

static void irqHandlerPortCD(uint32_t portCISFR, uint32_t portDISFR)
{
	if (portDISFR & 1) {
		//debugLEDOn();
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
	sysTickDelayMs(5);

	uint8_t whoAmI = i2cChannelReadByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_WHOAMI);

	if (whoAmI == FXOS8700CQ_WHOAMI_VAL) {
		SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
		portInitialise(&portDPins);
		FGPIOD_PDDR &= ~(3);

		// Set up transient acceleration interrupt on all axes with 100Hz read rate, 0.18g threshold, debounced to 50ms
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, 0);
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_TRANSIENT_CFG,
							FXOS8700CQ_TRANS_CFG_ELE_MASK|FXOS8700CQ_TRANS_CFG_ZEFE_MASK|FXOS8700CQ_TRANS_CFG_YEFE_MASK|FXOS8700CQ_TRANS_CFG_XEFE_MASK);
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_TRANSIENT_THS, FXOS8700CQ_TRANS_THS_THS(3));
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_TRANSIENT_COUNT, 5);
		// Transient interrupt enabled on INT1 output (MCU input D0)
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG4, FXOS8700CQ_INT_TRANS_MASK);
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG5, FXOS8700CQ_INT_TRANS_MASK);

		interruptRegisterPortCDIRQHandler(irqHandlerPortCD);
		NVIC_EnableIRQ(PORTC_PORTD_IRQn);
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, FXOS8700CQ_CTRL_REG1_ODR(3)|FXOS8700CQ_CTRL_REG1_ACTIVE_MASK);

		accelInitialised = 1;
	}
}

int accelCheckTransientInterrupt()
{
	return accelerometerInt1Flag > 0;
}

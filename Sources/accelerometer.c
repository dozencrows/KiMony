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
#include "systick.h"

#define FXOS8700CQ_I2C_CHANNEL	0
#define FXOS8700CQ_SLAVE_ADDR	0x1d

#define FXOS8700CQ_STATUS		0x00
#define FXOS8700CQ_WHOAMI		0x0D
#define FXOS8700CQ_XYZ_DATA_CFG	0x0E
#define FXOS8700CQ_CTRL_REG1	0x2A
#define FXOS8700CQ_M_CTRL_REG1	0x5B
#define FXOS8700CQ_M_CTRL_REG2	0x5C

#define FXOS8700CQ_WHOAMI_VAL	0xC7
#define FXOS8700CQ_PACKET_SIZE	13

typedef struct _SensorData {
	int16_t	x, y, z;
} SensorData;

static int accelInitialised = 0;

static const PortConfig portDPins =
{
	PORTE_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(1),
	2,
	{ 0, 1 }
};

void accelInit()
{
	uint8_t whoAmI = i2cChannelReadByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_WHOAMI);

	if (whoAmI == FXOS8700CQ_WHOAMI_VAL) {
		SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
		portInitialise(&portDPins);

		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, 0x00);		// place in standby
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_M_CTRL_REG1, 0x1f);	// Magnetometer setup 1
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_M_CTRL_REG2, 0x20);	// Magnetometer setup 2
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_XYZ_DATA_CFG, 0x01);	// Data config
		i2cChannelSendByte(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, 0x0d);		// Set data rate, low noise, normal reads & enable

		accelInitialised = 1;
	}
}

void accelTest()
{
	accelInit();

	if (accelInitialised) {
		uint8_t dataPacket[FXOS8700CQ_PACKET_SIZE];
		SensorData sensorData[2];

		while (1) {
			i2cChannelReadBlock(FXOS8700CQ_I2C_CHANNEL, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_STATUS, dataPacket, FXOS8700CQ_PACKET_SIZE);
			sensorData[0].x = (int16_t)((dataPacket[1] << 8) | dataPacket[2]) >> 2;
			sensorData[0].y = (int16_t)((dataPacket[3] << 8) | dataPacket[4]) >> 2;
			sensorData[0].z = (int16_t)((dataPacket[5] << 8) | dataPacket[6]) >> 2;

			printf("%02x: %d %d %d\n", dataPacket[0], sensorData[0].x, sensorData[0].y, sensorData[0].z);
		}
	}
}

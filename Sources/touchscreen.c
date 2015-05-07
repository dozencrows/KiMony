/*
 * touchscreen.c
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#include "touchscreen.h"

#include "MKL26Z4.h"
#include <stdio.h>

#include "spi.h"
#include "port_util.h"

// Pins to initialise
// PTD: 2 (Interrupt)
//		4 (Chip Select)

static const PortConfig portDPins =
{
	PORTD_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(1),
	2,
	{ 2, 4 }
};

void touchScreenGetSamples(uint8_t cmd1, uint8_t cmd2, uint16_t* buffer, uint16_t count)
{
	uint8_t data[2];

	// Ignore first sample
	spiWrite(cmd1);
	spiRead();
	spiRead();

	while (count-- > 1) {
		spiWrite(cmd1);
		data[0] = spiRead();
		data[1] = spiRead();

		*buffer++ = (uint16_t)(data[0] & 0x7f) << 5 | (data[1] >> 3);
	}

	spiWrite(cmd2);
	data[0] = spiRead();
	data[1] = spiRead();

	*buffer++ = (uint16_t)(data[0] & 0x7f) << 5 | (data[1] >> 3);
}

#define XPT2046_START		0x80
#define XPT2046_ADDR_X		0x10
#define XPT2046_ADDR_Y		0x50
#define XPT2046_MODE_12BIT	0x00
#define XPT2046_MODE_8BIT	0x08
#define XPT2046_PD1			0x02
#define XPT2046_PD0			0x01

#define TS_GETX_1	(XPT2046_START|XPT2046_ADDR_X|XPT2046_MODE_12BIT|XPT2046_PD1|XPT2046_PD0)
#define TS_GETX_2	(XPT2046_START|XPT2046_ADDR_X|XPT2046_MODE_12BIT|XPT2046_PD1|XPT2046_PD0)
#define TS_GETY_1	(XPT2046_START|XPT2046_ADDR_Y|XPT2046_MODE_12BIT|XPT2046_PD1|XPT2046_PD0)
#define TS_GETY_2	(XPT2046_START|XPT2046_ADDR_Y|XPT2046_MODE_12BIT)

void touchScreenInit()
{
	portInitialise(&portDPins);
	FGPIOD_PDDR &= ~(1 << 2);
	FGPIOD_PDDR |= (1 << 4);
	FGPIOD_PSOR = (1 << 4);
}

void touchScreenTest()
{
	int touch_count = 5;

	while(touch_count > 0) {
		printf("Waiting for touch...\n");
		uint32_t port_data;
		do {
			port_data = FGPIOD_PDIR;
		} while (port_data & (1 << 2));

		touch_count--;

		uint16_t x_buffer[6], y_buffer[6];

		FGPIOD_PCOR = (1 << 4);
		touchScreenGetSamples(TS_GETX_1, TS_GETX_2, x_buffer, 6);
		touchScreenGetSamples(TS_GETY_1, TS_GETY_2, y_buffer, 6);
		FGPIOD_PSOR = (1 << 4);

		printf("Touch!\n");
		printf("%d %d %d %d %d %d: %d %d %d %d %d %d\n", x_buffer[0], x_buffer[1], x_buffer[2], x_buffer[3], x_buffer[4], x_buffer[5],
											  	  	     y_buffer[0], y_buffer[1], y_buffer[2], y_buffer[3], y_buffer[4], y_buffer[5]);
	}
}

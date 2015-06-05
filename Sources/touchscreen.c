//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

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
#include "ports.h"
#include "interrupts.h"
#include "renderer.h"
#include "mathutil.h"
#include "systick.h"

// 3 and 8 are 2MHz, 4 and 8 are 1.5MHz, 5 and 8 are 1.2MHz, 6 and 8 are 1MHz
#define SPI_BR_PRESCALE	3
#define SPI_BR_DIVISOR	8

#define MIN_PRESSURE_THRESHOLD	500

// Pins to initialise
// PTD: 2 (Interrupt)
//		4 (Chip Select)

static const PortConfig portDPins =
{
	PORTD_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK | PORT_PCR_IRQC_MASK),
	PORT_PCR_MUX(1),
	2,
	{ 2, 4 }
};

static const PortConfig portDPinInterrupts =
{
	PORTD_BASE_PTR,
	~(PORT_PCR_IRQC_MASK),
	PORT_PCR_IRQC(0x0a),		// falling edge interrupt
	1,
	{ 2 }
};

static volatile uint8_t touchScreenIntFlag = 0;

static void irqHandlerPortCD(uint32_t portCISFR, uint32_t portDISFR)
{
	if (portDISFR & (1 << 2)) {
		touchScreenIntFlag++;
	}
}

static void touchScreenGetSamples(uint8_t cmd1, uint8_t cmd2, uint16_t* buffer, uint16_t count)
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
#define XPT2046_ADDR_Z1		0x30
#define XPT2046_ADDR_Z2		0x40
#define XPT2046_MODE_12BIT	0x00
#define XPT2046_MODE_8BIT	0x08
#define XPT2046_PD1			0x02
#define XPT2046_PD0			0x01

#define TS_GETX_1	(XPT2046_START|XPT2046_ADDR_X|XPT2046_MODE_12BIT|XPT2046_PD0)
#define TS_GETX_2	(XPT2046_START|XPT2046_ADDR_X|XPT2046_MODE_12BIT|XPT2046_PD0)
#define TS_GETY_1	(XPT2046_START|XPT2046_ADDR_Y|XPT2046_MODE_12BIT|XPT2046_PD0)
#define TS_GETY_2	(XPT2046_START|XPT2046_ADDR_Y|XPT2046_MODE_12BIT)
#define TS_GETZ1_1	(XPT2046_START|XPT2046_ADDR_Z1|XPT2046_MODE_12BIT|XPT2046_PD0)
#define TS_GETZ1_2	(XPT2046_START|XPT2046_ADDR_Z1|XPT2046_MODE_12BIT|XPT2046_PD0)
#define TS_GETZ2_1	(XPT2046_START|XPT2046_ADDR_Z2|XPT2046_MODE_12BIT|XPT2046_PD0)
#define TS_GETZ2_2	(XPT2046_START|XPT2046_ADDR_Z2|XPT2046_MODE_12BIT|XPT2046_PD0)

void touchScreenInit()
{
	portInitialise(&portDPins);
	portInitialise(&portDPinInterrupts);
	FGPIOD_PDDR &= ~(1 << 2);
	FGPIOD_PDDR |= (1 << 4);
	FGPIOD_PSOR = (1 << 4);
	interruptRegisterPortCDIRQHandler(irqHandlerPortCD);
	NVIC_EnableIRQ(PORTC_PORTD_IRQn);
}

void touchScreenClearInterrupt()
{
	PORTD_ISFR = 1 << 2;
	touchScreenIntFlag = 0;
}

int touchScreenCheckInterrupt()
{
	return touchScreenIntFlag;
}

static void waitForTouch()
{
	touchScreenClearInterrupt();
	uint8_t lastIntFlag = 0;
	do {
		lastIntFlag = touchScreenIntFlag;
	} while (!lastIntFlag);

	uint8_t nextIntFlag = lastIntFlag;
	do {
		lastIntFlag = nextIntFlag;
		sysTickDelayMs(8);
		nextIntFlag = touchScreenIntFlag;
	} while (nextIntFlag != lastIntFlag);
}

static void getRawTouch(uint16_t* touchData, uint16_t* pressureData, uint16_t sampleCount)
{
	NVIC_DisableIRQ(PORTC_PORTD_IRQn);
	FGPIOD_PCOR = (1 << 4);
	touchScreenGetSamples(TS_GETZ1_1, TS_GETZ1_2, pressureData, sampleCount);
	touchScreenGetSamples(TS_GETZ2_1, TS_GETZ2_2, pressureData + sampleCount, sampleCount);
	touchScreenGetSamples(TS_GETX_1, TS_GETX_2, touchData, sampleCount);
	touchScreenGetSamples(TS_GETY_1, TS_GETY_2, touchData + sampleCount, sampleCount);
	FGPIOD_PSOR = (1 << 4);
	touchScreenIntFlag = 0;
	// Ensure no pending interrupt
	PORTD_ISFR = 1 << 2;
	NVIC_EnableIRQ(PORTC_PORTD_IRQn);
}

#define FMED_SORT(a,b) { if ((a)>(b)) { SWAPNUM(a, b); } }

static uint16_t fastMedian5(uint16_t* values)
{
	FMED_SORT(values[0], values[1]);
	FMED_SORT(values[3], values[4]);
	FMED_SORT(values[0], values[3]);
	FMED_SORT(values[1], values[4]);
	FMED_SORT(values[1], values[2]);
	FMED_SORT(values[2], values[3]);
	FMED_SORT(values[1], values[2]);
    return(values[2]) ;
}

static void renderCross(uint16_t x, uint16_t y, uint16_t size, uint16_t colour)
{
	int xclip = x - (size / 2);
	if (xclip < 0) {
		rendererDrawHLine(0, y, size + xclip, colour);
	}
	else {
		rendererDrawHLine(x - size / 2, y, size, colour);
	}

	int yclip = y - (size / 2);
	if (yclip < 0) {
		rendererDrawVLine(x, 0, size + yclip, colour);
	}
	else {
		rendererDrawVLine(x, y - size / 2, size, colour);
	}
}

typedef struct _CalibrationSample {
	uint16_t screenX, screenY;
	uint16_t touchX, touchY;
} CalibrationSample;

#define CALIBRATION_SAMPLE_COUNT	4

CalibrationSample calibrationSamples[CALIBRATION_SAMPLE_COUNT] = {
	{ (SCREEN_WIDTH * 10) / 100, (SCREEN_HEIGHT * 10) / 100, 0, 0 },
	{ (SCREEN_WIDTH * 50) / 100, (SCREEN_HEIGHT * 90) / 100, 0, 0 },
	{ (SCREEN_WIDTH * 90) / 100, (SCREEN_HEIGHT * 50) / 100, 0, 0 },
	{ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0, 0 }
};

void touchScreenCalibrate()
{
	uint16_t touchData[10];
	uint16_t pressureData[10];

	spiSetBitRate(SPI_BR_PRESCALE, SPI_BR_DIVISOR);

	for (int i = 0; i < CALIBRATION_SAMPLE_COUNT; i++) {
		rendererClearScreen();
		rendererNewDrawList();
		renderCross(calibrationSamples[i].screenX, calibrationSamples[i].screenY, 15, 0xffff);
		rendererRenderDrawList();
		waitForTouch();
		getRawTouch(touchData, pressureData, 5);
		calibrationSamples[i].touchX = fastMedian5(touchData);
		calibrationSamples[i].touchY = fastMedian5(touchData + 5);

		printf("(%d, %d) -> (%d, %d)\n", calibrationSamples[i].screenX, calibrationSamples[i].screenY,
				calibrationSamples[i].touchX, calibrationSamples[i].touchY);
	}
}

#define FIXED_POINT_SHIFT	16
#define ROUND_OFFSET 		(1 << (FIXED_POINT_SHIFT - 1))

// These are 16.16 fixed point values, determined from calibration (via spreadsheet)
const int32_t alphaX 	= -33;
const int32_t betaX 	= -4248;
const int32_t alphaY 	= -5857;
const int32_t betaY		= -3;

const int32_t deltaX 	= 254;
const int32_t deltaY 	= 331;

static void getCalibratedPoint(uint16_t* pIn, uint16_t* pOut)
{
	int32_t rX = (((int32_t)pIn[0] * alphaX) + ((int32_t)pIn[1] * betaX) + ROUND_OFFSET) >> FIXED_POINT_SHIFT;
	int32_t rY = (((int32_t)pIn[0] * alphaY) + ((int32_t)pIn[1] * betaY) + ROUND_OFFSET) >> FIXED_POINT_SHIFT;

	pOut[0] = (uint16_t)(MAX(rX + deltaX, 0));
	pOut[1] = (uint16_t)(MAX(rY + deltaY, 0));
}

int touchScreenGetCoordinates(Point* p)
{
	uint16_t touchData[10];
	uint16_t pressureData[10];
	uint16_t touchPt[2];
	uint16_t touchZ[2];

	spiSetBitRate(SPI_BR_PRESCALE, SPI_BR_DIVISOR);
	getRawTouch(touchData, pressureData, 5);

	touchPt[0] = fastMedian5(touchData);
	touchPt[1] = fastMedian5(touchData + 5);
	touchZ[0] = MAX(fastMedian5(pressureData), 1);
	touchZ[1] = fastMedian5(pressureData + 5);

	getCalibratedPoint(touchPt, &p->x);
	// Approximate pressure measure for noise filtering
	int pressure = 4096 - (touchZ[1] - touchZ[0]);

	return !(FGPIOD_PDIR & (1 << 2)) && pressure > MIN_PRESSURE_THRESHOLD;
}

void touchScreenTest()
{
	Point touch;

	rendererClearScreen();
	spiSetBitRate(SPI_BR_PRESCALE, SPI_BR_DIVISOR);

	while(1) {
		if (!(FGPIOD_PDIR & (1 << 2))) {
			if (touchScreenGetCoordinates(&touch)) {
				rendererNewDrawList();
				rendererDrawHLine(touch.x, touch.y, 1, 0xffff);
				rendererRenderDrawList();
			}
		}
	}
}

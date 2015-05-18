/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include "MKL26Z4.h"
#include "systick.h"
#include "i2c.h"
#include "spi.h"

#include "keymatrix.h"
#include "lcd.h"
#include "touchscreen.h"
#include "ir.h"
#include "flash.h"
#include "renderer.h"
#include "mathutil.h"
#include "interrupts.h"

#define IRCODE_NOP	0
#define IRCODE_RC6	1
#define IRCODE_SIRC	2

typedef struct _IrCode {
	unsigned int encoding:4;
	unsigned int bits:5;
	unsigned int code:23;
	uint32_t toggleMask;
} IrCode;

typedef struct _IrAction {
	int		codeCount;
	IrCode	codes[];
} IrAction;

typedef struct _ButtonMapping {
	uint32_t		buttonMask;
	const IrAction*	action;
} ButtonMapping;

static const IrAction powerOnOff =
{
	4,
	{
		{ IRCODE_RC6, 	21,	0xFFB38,	0 	},
		{ IRCODE_RC6,	21,	0xEFB38,	0 	},
		{ IRCODE_NOP,	0,	250, 		0 	},
		{ IRCODE_SIRC, 	12,	0xA90, 		0	}
	}
};

static const IrAction numeric1 = { 1, { IRCODE_SIRC, 12, 0x010, 0 } };
static const IrAction numeric2 = { 1, { IRCODE_SIRC, 12, 0x810, 0 } };
static const IrAction numeric3 = { 1, { IRCODE_SIRC, 12, 0x410, 0 } };
static const IrAction numeric4 = { 1, { IRCODE_SIRC, 12, 0xC10, 0 } };
static const IrAction numeric5 = { 1, { IRCODE_SIRC, 12, 0x210, 0 } };
static const IrAction numeric6 = { 1, { IRCODE_SIRC, 12, 0xA10, 0 } };
static const IrAction numeric7 = { 1, { IRCODE_SIRC, 12, 0x610, 0 } };
static const IrAction numeric8 = { 1, { IRCODE_SIRC, 12, 0xE10, 0 } };
static const IrAction numeric9 = { 1, { IRCODE_SIRC, 12, 0x110, 0 } };
static const IrAction numeric0 = { 1, { IRCODE_SIRC, 12, 0x910, 0 } };

static const IrAction volumeUp 		= { 1, { IRCODE_RC6, 21, 0xEEFEF, 0x10000 } };
static const IrAction volumeDown 	= { 1, { IRCODE_RC6, 21, 0xEEFEE, 0x10000 } };
static const IrAction mute 			= { 1, { IRCODE_RC6, 21, 0xEEFF2, 0x10000 } };
static const IrAction surround		= { 1, { IRCODE_RC6, 21, 0xEEFAD, 0x10000 } };

static const IrAction channelUp 	= { 1, { IRCODE_SIRC, 12, 0x090, 0 } };
static const IrAction channelDown	= { 1, { IRCODE_SIRC, 12, 0x890, 0 } };
static const IrAction info			= { 1, { IRCODE_SIRC, 12, 0x5D0, 0 } };

static const ButtonMapping buttonMappings[] =
{
	{ 0x020000, &powerOnOff },
	{ 0x008000, &numeric1 },
	{ 0x000800, &numeric2 },
	{ 0x000080, &numeric3 },
	{ 0x004000, &numeric4 },
	{ 0x000400, &numeric5 },
	{ 0x000040, &numeric6 },
	{ 0x002000, &numeric7 },
	{ 0x000200, &numeric8 },
	{ 0x000020, &numeric9 },
	{ 0x000100, &numeric0 },
	{ 0x000008, &volumeUp },
	{ 0x000004, &volumeDown },
	{ 0x000002, &channelUp },
	{ 0x000001, &channelDown },
	{ 0x000010, &mute },
	{ 0x001000, &surround },
	{ 0x200000, &info },
};

static uint32_t toggleBit = 0;

void performIrAction(const IrAction* action)
{
	const IrCode* code = action->codes;

	for (int i = 0; i < action->codeCount; i++, code++) {
		if (code->toggleMask) {
			toggleBit ^= code->toggleMask;
		}
		switch (code->encoding) {
			case IRCODE_NOP: {
				sysTickDelayMs(code->code);
				break;
			}
			case IRCODE_RC6: {
				irSendRC6Code(code->code|toggleBit, code->bits);
				break;
			}
			case IRCODE_SIRC: {
				irSendSIRCCode(code->code|toggleBit, code->bits);
				break;
			}
		}
	}
}

void waitForButton()
{
	uint32_t lastKeys = keyMatrixPoll();

	while (1) {
		uint32_t keys = keyMatrixPoll();
		uint32_t change = keys ^ lastKeys;
		if (keys & change) {
			lastKeys = keys;
			break;
		}
		else {
			sysTickDelayMs(100);
		}
	}
}

static volatile uint8_t pitIrqCount = 0;

static void pitIrqHandler()
{
	pitIrqCount++;
}

void mainLoop()
{
	uint16_t keyColourM = 0xf81f;
	uint16_t keyColourNM = 0xf81f;

	// Periodic timer for polling non-matrix keys
	SIM_SCGC6   |= SIM_SCGC6_PIT_MASK;
	PIT_MCR 	= PIT_MCR_FRZ_MASK;
	PIT_LDVAL0	= DEFAULT_SYSTEM_CLOCK / 200;				// 100Hz (uses bus clock, which is half system clock)
	PIT_TCTRL0	= PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
	interruptRegisterPITIRQHandler(pitIrqHandler);
	NVIC_EnableIRQ(PIT_IRQn);

	uint32_t keypadState = keyMatrixPoll() | keyNonMatrixPoll();

	keyMatrixClearInterrupt();
	touchScreenClearInterrupt();

	while (1) {
		__asm("wfi");

		uint32_t keypadNewState = keypadState;
		const IrAction* action = NULL;

		rendererNewDrawList();

		if (touchScreenCheckInterrupt()) {
			Point touch;
			if (touchScreenGetCoordinates(&touch)) {
				rendererDrawHLine(touch.x, touch.y, 1, 0xffff);
			}
			touchScreenClearInterrupt();
		}

		if (keyMatrixCheckInterrupt()) {
			keypadNewState = keyMatrixPoll() | (keypadNewState & KEY_NONMATRIX_MASK);
			keyMatrixClearInterrupt();
		}

		if (pitIrqCount) {
			keypadNewState = keyNonMatrixPoll() | (keypadNewState & KEY_MATRIX_MASK);
			pitIrqCount = 0;
		}

		uint32_t keypadChange = keypadState ^ keypadNewState;

		if (keypadChange) {
			uint32_t keypadNewOn = keypadNewState & keypadChange;
			if (keypadNewOn) {
				rendererDrawRect(119, 161, 2, 2, keyColourNM);
				keyColourNM ^= 0xffff;

				for (size_t i = 0; i < sizeof(buttonMappings) / sizeof(buttonMappings[0]); i++) {
					if (buttonMappings[i].buttonMask == keypadNewOn) {
						action = buttonMappings[i].action;
						break;
					}
				}
			}

			keypadState = keypadNewState;
		}

		rendererRenderDrawList();

		if (action) {
			performIrAction(action);
		}
	}
}

int main(void)
{
	SystemCoreClockUpdate();

	sysTickInit();
	sysTickSetClockRate(SystemCoreClock);

	i2cInit();
	spiInit();

	tftInit();
	tftSetBacklight(1);
	rendererInit();
	rendererClearScreen();

	touchScreenInit();
	keyMatrixInit();
	flashInit();

	mainLoop();

	//irTest();
	//flashTest();
	//testKeyMatrix();
	//touchScreenTest();
	//touchScreenCalibrate();
	//rendererTest();

//	int frames = 0;
//	sysTickEventInMs(1000);
//	while (!sysTickCheckEvent()) {
//		drawTestRectDma();
//		frames++;
//	}
//
//	printf("DMA %d\n", frames);
//	waitForButton();
//	drawTestRect_PEInline_FGPIO(0);
//
//	frames = 0;
//	sysTickEventInMs(1000);
//	while (!sysTickCheckEvent()) {
//		drawTestRect_PEInline_FGPIO(0x1ff8);
//		frames++;
//	}
//
//	printf("FGPIO %d\n", frames);
//	waitForButton();
//	drawTestRect_PEInline_FGPIO(0);
//
//	frames = 0;
//	sysTickEventInMs(1000);
//	while (!sysTickCheckEvent()) {
//		drawTestRect_PEInline_BME(0x1ff8);
//		frames++;
//	}
//
//	printf("BME %d\n", frames);
//	waitForButton();
//	drawTestRect_PEInline_FGPIO(0);
//
//	frames = 0;
//	sysTickEventInMs(1000);
//	while (!sysTickCheckEvent()) {
//		drawTestRect_PEInline_SRAM(0x1ff8);
//		frames++;
//	}
//
//	printf("FGPIO SRAM %d\n", frames);
//	waitForButton();
//	drawTestRect_PEInline_FGPIO(0);
//
//	frames = 0;
//	sysTickEventInMs(1000);
//	while (!sysTickCheckEvent()) {
//		drawTestRect_PEInline_SRAM_PDOR(0x1ff8);
//		frames++;
//	}
//
//	printf("FGPIO SRAM PDOR %d\n", frames);
//	waitForButton();
//	drawTestRect_PEInline_FGPIO(0);
//
//	frames = 0;
//	sysTickEventInMs(1000);
//	while (!sysTickCheckEvent()) {
//		drawTestRect_PEInline_SRAM_PDOR_BufferFill(0x1ff8);
//		frames++;
//	}
//
//	printf("FGPIO SRAM PDOR BufferFill %d\n", frames);
//	waitForButton();
//	drawTestRect_PEInline_FGPIO(0);
//
//	frames = 0;
//	sysTickEventInMs(1000);
//	while (!sysTickCheckEvent()) {
//		drawTestRect_PEInline_WROnly(0x1ff8);
//		frames++;
//	}
//
//	printf("WROnly %d\n", frames);
//	waitForButton();
//	drawTestRect_PEInline_FGPIO(0);
//
//	frames = 0;
//	sysTickEventInMs(1000);
//	while (!sysTickCheckEvent()) {
//		drawTestImage(0, 0);
//		drawTestImage(120, 0);
//		drawTestImage(0, 160);
//		drawTestImage(120, 160);
//		frames++;
//	}
//
//	printf("FGPIO Flash %d\n", frames);
//
//	for(;;) {
//	}

    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

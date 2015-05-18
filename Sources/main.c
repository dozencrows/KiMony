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
} IrCode;

typedef struct _IrAction {
	int		codeCount;
	IrCode	codes[];
} IrAction;

static const IrAction powerOnOff =
{
	4,
	{
		{ IRCODE_RC6, 	21,	0xFFB38 },
		{ IRCODE_RC6,	21,	0xEFB38 },
		{ IRCODE_NOP,	0,	250 	},
		{ IRCODE_SIRC, 	12,	0xA90 	}
	}
};

void performIrAction(const IrAction* action)
{
	const IrCode* code = action->codes;

	for (int i = 0; i < action->codeCount; i++, code++) {
		switch (code->encoding) {
			case IRCODE_NOP: {
				sysTickDelayMs(code->code);
				break;
			}
			case IRCODE_RC6: {
				irSendRC6Code(code->code, code->bits);
				break;
			}
			case IRCODE_SIRC: {
				irSendSIRCCode(code->code, code->bits);
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
			if (keypadNewState & keypadChange) {
				rendererDrawRect(119, 161, 2, 2, keyColourNM);
				keyColourNM ^= 0xffff;
				performIrAction(&powerOnOff);
			}

			keypadState = keypadNewState;
		}

		rendererRenderDrawList();
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

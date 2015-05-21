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
#include "interrupts.h"
#include "mathutil.h"

#include "keymatrix.h"
#include "lcd.h"
#include "touchscreen.h"
#include "ir.h"
#include "flash.h"
#include "renderer.h"
#include "buttons.h"
#include "touchbuttons.h"

// Time until backlight turns off when idle, in hundredths of a second
#define BACKLIGHT_OFF_TIMEOUT	500

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

static const IrAction red 		= { 1, { IRCODE_SIRC, 15, 0x52E9, 0 } };
static const IrAction yellow 	= { 1, { IRCODE_SIRC, 15, 0x72E9, 0 } };
static const IrAction green 	= { 1, { IRCODE_SIRC, 15, 0x32E9, 0 } };
static const IrAction blue	 	= { 1, { IRCODE_SIRC, 15, 0x12E9, 0 } };

static const IrAction guide 		= { 1, { IRCODE_SIRC, 15, 0x6D25, 0 } };
static const IrAction enter 		= { 1, { IRCODE_SIRC, 12, 0xA70, 0 } };
static const IrAction back 			= { 1, { IRCODE_SIRC, 12, 0xC70, 0 } };
static const IrAction home 			= { 1, { IRCODE_SIRC, 12, 0x070, 0 } };

static const ButtonMapping buttonMappings[] =
{
	{ 0x200000, &info },
	{ 0x100000, &blue },
	{ 0x080000, &green },
	{ 0x040000, &yellow },
	{ 0x020000, &powerOnOff },
	{ 0x010000, &red },
	{ 0x008000, &numeric1 },
	{ 0x000800, &numeric2 },
	{ 0x000080, &numeric3 },
	{ 0x000008, &volumeUp },
	{ 0x004000, &numeric4 },
	{ 0x000400, &numeric5 },
	{ 0x000040, &numeric6 },
	{ 0x000004, &volumeDown },
	{ 0x002000, &numeric7 },
	{ 0x000200, &numeric8 },
	{ 0x000020, &numeric9 },
	{ 0x000002, &channelUp },
	{ 0x001000, &surround },
	{ 0x000100, &numeric0 },
	{ 0x000010, &mute },
	{ 0x000001, &channelDown },
};

#define BUTTON_COLUMNS	4
#define BUTTON_ROWS		6

#define BUTTON_WIDTH	(SCREEN_WIDTH/BUTTON_COLUMNS)
#define BUTTON_HEIGHT	(SCREEN_HEIGHT/BUTTON_ROWS)

static const TouchButton touchButtons[] =
{
	{ &guide,   		   0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ &enter,   BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ &back,  2*BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ &home,  3*BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
};

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

static uint32_t backlightCounter = 0;

static void backlightOn()
{
	backlightCounter = 0;
	tftSetBacklight(1);
}

void mainLoop()
{
	buttonsInit();
	buttonsSetActiveMapping(buttonMappings, sizeof(buttonMappings) / sizeof(buttonMappings[0]));

	touchbuttonsInit();
	touchbuttonsSetActive(touchButtons, sizeof(touchButtons) / sizeof(touchButtons[0]));

	// Periodic timer for polling non-matrix keys
	SIM_SCGC6   |= SIM_SCGC6_PIT_MASK;
	PIT_MCR 	= PIT_MCR_FRZ_MASK;
	PIT_LDVAL0	= DEFAULT_SYSTEM_CLOCK / 200;				// 100Hz (uses bus clock, which is half system clock)
	PIT_TCTRL0	= PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
	interruptRegisterPITIRQHandler(pitIrqHandler);
	NVIC_EnableIRQ(PIT_IRQn);

	keyMatrixClearInterrupt();
	touchScreenClearInterrupt();

	while (1) {
		__asm("wfi");

		const IrAction* action = NULL;

		rendererNewDrawList();

		if (touchScreenCheckInterrupt()) {
			Point touch;
			if (touchScreenGetCoordinates(&touch)) {
				if (tftGetBacklight()) {
					touchbuttonsProcessTouch(&touch, &action);
				}
				backlightOn();
			}
			touchScreenClearInterrupt();
		}

		if (keyMatrixCheckInterrupt()) {
			buttonsPollState();
			keyMatrixClearInterrupt();
		}

		if (pitIrqCount) {
			buttonsPollState();
			touchButtonsUpdate();
			pitIrqCount = 0;

			backlightCounter++;
			if (backlightCounter > BACKLIGHT_OFF_TIMEOUT) {
				tftSetBacklight(0);
				backlightCounter = 0;
			}
		}

		buttonsUpdate(&action);
		touchbuttonsRender();

		if (action) {
			backlightOn();
			rendererRenderDrawList();
			irDoAction(action);
		}
		else {
			rendererRenderDrawList();
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

	//flashTest();
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

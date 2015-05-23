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
#include "activity.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

// Time until backlight turns off when idle, in hundredths of a second
#define BACKLIGHT_OFF_TIMEOUT	500

static const Event homeActivityEvent = { EVENT_HOME, NULL };
static const Event nextPageEvent = { EVENT_NEXTPAGE, NULL };
static const Event prevPageEvent = { EVENT_PREVPAGE, NULL };

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

static const Event numeric1Event 		= { EVENT_IRACTION, &numeric1 };
static const Event numeric2Event 		= { EVENT_IRACTION, &numeric2 };
static const Event numeric3Event 		= { EVENT_IRACTION, &numeric3 };
static const Event numeric4Event 		= { EVENT_IRACTION, &numeric4 };
static const Event numeric5Event 		= { EVENT_IRACTION, &numeric5 };
static const Event numeric6Event 		= { EVENT_IRACTION, &numeric6 };
static const Event numeric7Event 		= { EVENT_IRACTION, &numeric7 };
static const Event numeric8Event 		= { EVENT_IRACTION, &numeric8 };
static const Event numeric9Event 		= { EVENT_IRACTION, &numeric9 };
static const Event numeric0Event 		= { EVENT_IRACTION, &numeric0 };

static const Event volumeUpEvent 		= { EVENT_IRACTION, &volumeUp };
static const Event volumeDownEvent 		= { EVENT_IRACTION, &volumeDown };
static const Event muteEvent 			= { EVENT_IRACTION, &mute };
static const Event surroundEvent		= { EVENT_IRACTION, &surround };

static const Event channelUpEvent 		= { EVENT_IRACTION, &channelUp };
static const Event channelDownEvent		= { EVENT_IRACTION, &channelDown };
static const Event infoEvent			= { EVENT_IRACTION, &info };

static const Event redEvent 			= { EVENT_IRACTION, &red };
static const Event yellowEvent 			= { EVENT_IRACTION, &yellow };
static const Event greenEvent 			= { EVENT_IRACTION, &green };
static const Event blueEvent	 		= { EVENT_IRACTION, &blue };

static const Event guideEvent 			= { EVENT_IRACTION, &guide };
static const Event enterEvent 			= { EVENT_IRACTION, &enter };
static const Event backEvent 			= { EVENT_IRACTION, &back };
static const Event homeEvent 			= { EVENT_IRACTION, &home };

static const Event powerOnOffEvent 		= { EVENT_IRACTION, &powerOnOff };

static const ButtonMapping testRemoteButtonMappings[] =
{
	{ 0x200000, &infoEvent },
	//{ 0x100000, &blueEvent },
	//{ 0x080000, &greenEvent },
	//{ 0x040000, &yellowEvent },
	{ 0x020000, &powerOnOffEvent },
	{ 0x010000, &homeActivityEvent },
	{ 0x008000, &numeric1Event },
	{ 0x000800, &numeric2Event },
	{ 0x000080, &numeric3Event },
	{ 0x000008, &volumeUpEvent },
	{ 0x004000, &numeric4Event },
	{ 0x000400, &numeric5Event },
	{ 0x000040, &numeric6Event },
	{ 0x000004, &volumeDownEvent },
	{ 0x002000, &numeric7Event },
	{ 0x000200, &numeric8Event },
	{ 0x000020, &numeric9Event },
	{ 0x000002, &channelUpEvent },
	{ 0x001000, &surroundEvent },
	{ 0x000100, &numeric0Event },
	{ 0x000010, &muteEvent },
	{ 0x000001, &channelDownEvent },
};

#define BUTTON_COLUMNS	4
#define BUTTON_ROWS		6

#define BUTTON_WIDTH	(SCREEN_WIDTH/BUTTON_COLUMNS)
#define BUTTON_HEIGHT	(SCREEN_HEIGHT/BUTTON_ROWS)

static const TouchButton testRemoteTouchButtons[] =
{
	{ &guideEvent,   		    0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ &enterEvent,   BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ &backEvent,  2*BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ &homeEvent,  3*BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },

	{ &redEvent,    	          0, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf800 },
	{ &greenEvent,     BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0x07e0 },
	{ &yellowEvent,  2*BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xffe0 },
	{ &blueEvent,    3*BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0x001f },

	{ NULL,   	          0, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,    BUTTON_WIDTH, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,  2*BUTTON_WIDTH, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,  3*BUTTON_WIDTH, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },

	{ NULL,   	          0, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,    BUTTON_WIDTH, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,  2*BUTTON_WIDTH, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,  3*BUTTON_WIDTH, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },

	{ NULL,   	          0, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,    BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,  2*BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,  3*BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },

	{ NULL,   	          0, 5*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,    BUTTON_WIDTH, 5*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,  2*BUTTON_WIDTH, 5*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
	{ NULL,  3*BUTTON_WIDTH, 5*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 },
};

static const TouchButtonPage testRemotePages = { ARRAY_LENGTH(testRemoteTouchButtons), testRemoteTouchButtons };
static const Activity testRemote = { ARRAY_LENGTH(testRemoteButtonMappings), testRemoteButtonMappings, 1, &testRemotePages };

static const ButtonMapping homeActivityButtonMappings[] =
{
	{ 0x010000, &prevPageEvent },
	{ 0x040000, &nextPageEvent },
};

static const Event selectTestRemote = { EVENT_ACTIVITY, &testRemote};
static const TouchButton homeTouchButtonsPage1[] = {
	{ &selectTestRemote, 0, 0, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 }
};
static const TouchButton homeTouchButtonsPage2[] = {
	{ &selectTestRemote, 0, 1*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 }
};
static const TouchButton homeTouchButtonsPage3[] = {
	{ &selectTestRemote, 0, 2*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0 }
};
static const TouchButtonPage homePages[] = {
	{ ARRAY_LENGTH(homeTouchButtonsPage1), homeTouchButtonsPage1 },
	{ ARRAY_LENGTH(homeTouchButtonsPage2), homeTouchButtonsPage2 },
	{ ARRAY_LENGTH(homeTouchButtonsPage3), homeTouchButtonsPage3 },
};
static const Activity homeActivity = { ARRAY_LENGTH(homeActivityButtonMappings), homeActivityButtonMappings, ARRAY_LENGTH(homePages), homePages};

static int touchPage = 0;
static const Activity* currentActivity = NULL;

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

void selectActivity(const Activity* activity)
{
	buttonsSetActiveMapping(activity->buttonMapping, activity->buttonMappingCount);

	const TouchButton* touchButtons = NULL;
	int touchButtonCount = 0;

	if (activity->touchButtonPageCount) {
		touchButtons = activity->touchButtonPages[0].touchButtons;
		touchButtonCount = activity->touchButtonPages[0].touchButtonCount;
	}

	touchbuttonsSetActive(touchButtons, touchButtonCount);
	rendererNewDrawList();
	rendererClearScreen();
	rendererRenderDrawList();
	currentActivity = activity;
	touchPage = 0;
}

void selectTouchPage(int page)
{
	if (page < 0) {
		page = currentActivity->touchButtonPageCount - 1;
	}
	else if (page >= currentActivity->touchButtonPageCount) {
		page = 0;
	}

	touchPage = page;

	if (page < currentActivity->touchButtonPageCount) {
		touchbuttonsSetActive(currentActivity->touchButtonPages[page].touchButtons, currentActivity->touchButtonPages[page].touchButtonCount);
	}
	else {
		touchbuttonsSetActive(NULL, 0);
	}

	rendererNewDrawList();
	rendererClearScreen();
	rendererRenderDrawList();
}

void mainLoop()
{
	buttonsInit();
	touchbuttonsInit();

	selectActivity(&homeActivity);

//	int frames = 0;
//	sysTickEventInMs(1000);
//	while (!sysTickCheckEvent()) {
//		rendererNewDrawList();
//		touchbuttonsRender();
//		rendererRenderDrawList();
//		frames++;
//	}
//
//	printf("TouchButtons %d\n", frames);

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

		const Event* event = NULL;

		rendererNewDrawList();

		if (touchScreenCheckInterrupt()) {
			Point touch;
			if (touchScreenGetCoordinates(&touch)) {
				if (tftGetBacklight()) {
					touchbuttonsProcessTouch(&touch, &event);
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

		buttonsUpdate(&event);
		touchbuttonsRender();

		if (event) {
			backlightOn();
			rendererRenderDrawList();
			if (event->type == EVENT_IRACTION) {
				irDoAction(event->irAction);
			}
			else if (event->type == EVENT_ACTIVITY) {
				selectActivity(event->activity);
			}
			else if (event->type == EVENT_HOME) {
				selectActivity(&homeActivity);
			}
			else if (event->type == EVENT_NEXTPAGE) {
				selectTouchPage(touchPage + 1);
			}
			else if (event->type == EVENT_PREVPAGE) {
				selectTouchPage(touchPage - 1);
			}
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
	for(;;) {
	}

    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

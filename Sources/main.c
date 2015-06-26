//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <stdio.h>
#include <string.h>
#include "MKL26Z4.h"
#include "systick.h"
#include "i2c.h"
#include "spi.h"
#include "interrupts.h"
#include "uart.h"
#include "timer.h"
#include "mathutil.h"
#include "codeutil.h"
#include "renderutils.h"

#include "keymatrix.h"
#include "lcd.h"
#include "touchscreen.h"
#include "ir.h"
#include "flash.h"
#include "renderer.h"
#include "buttons.h"
#include "touchbuttons.h"
#include "accelerometer.h"
#include "device.h"
#include "activity.h"
#include "remotedata.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

// Time until backlight turns off when idle, in hundredths of a second
#define BACKLIGHT_OFF_TIMEOUT	500

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

void turnOffAllDevices()
{
	renderMessage("Powering down...", 0xffff);
	deviceSetStatesParallel(NULL, 0);
}

void selectActivity(const Activity* activity)
{
	if (currentActivity != activity) {
		if (!(activity->flags & ACTIVITY_NODEVICES)) {
			renderMessage("Switching...", 0xffff);
		}

		buttonsSetActiveMapping((const ButtonMapping*)GET_FLASH_PTR(activity->buttonMappingOffset), activity->buttonMappingCount);

		const TouchButton* touchButtons = NULL;
		int touchButtonCount = 0;

		if (activity->touchButtonPageCount) {
			const TouchButtonPage* tbPage = (const TouchButtonPage*)GET_FLASH_PTR(activity->touchButtonPagesOffset);
			touchButtons = (const TouchButton*)GET_FLASH_PTR(tbPage->touchButtonOffset);
			touchButtonCount = tbPage->touchButtonCount;
		}

		touchbuttonsSetActive(touchButtons, touchButtonCount);
		currentActivity = activity;
		touchPage = 0;

		if (!(activity->flags & ACTIVITY_NODEVICES)) {
			deviceSetStatesParallel((const DeviceState*)GET_FLASH_PTR(activity->deviceStatesOffset), activity->deviceStateCount);
		}

		rendererClearScreen();
	}
}

void forceActivity(const Activity* activity)
{
	currentActivity = NULL;
	selectActivity(activity);
}

void selectTouchPage(int page)
{
	if (page < 0) {
		page = currentActivity->touchButtonPageCount - 1;
	}
	else if (page >= currentActivity->touchButtonPageCount) {
		page = 0;
	}

	if (page != touchPage) {
		touchPage = page;

		if (page < currentActivity->touchButtonPageCount) {
			const TouchButtonPage* tbPages = (const TouchButtonPage*)GET_FLASH_PTR(currentActivity->touchButtonPagesOffset);
			touchbuttonsSetActive((const TouchButton*)GET_FLASH_PTR(tbPages[page].touchButtonOffset), tbPages->touchButtonCount);
		}
		else {
			touchbuttonsSetActive(NULL, 0);
		}

		rendererClearScreen();
	}
}

const Activity* remoteInit()
{
	deviceInit();
	const RemoteDataHeader* dataHeader = (const RemoteDataHeader*)GET_FLASH_PTR(0);
	deviceSetActive((const Device*)GET_FLASH_PTR(dataHeader->devicesOffset), dataHeader->deviceCount);

	return (const Activity*)GET_FLASH_PTR(dataHeader->homeActivityOffset);
}

void mainLoop()
{
	buttonsInit();
	touchbuttonsInit();

	const Activity* homeActivity = remoteInit();
	selectActivity(homeActivity);

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
					touchbuttonsProcessTouch(&touch);
				}
				backlightOn();
			}
			touchScreenClearInterrupt();
		}

		if (keyMatrixCheckInterrupt()) {
			buttonsPollState();
			keyMatrixClearInterrupt();
		}

		if (accelCheckTransientInterrupt()) {
			accelClearInterrupts();
			backlightOn();
		}

		if (pitIrqCount) {
			buttonsPollState();
			touchButtonsUpdate(&event);
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
				deviceDoIrAction((const Device*)GET_FLASH_PTR(event->deviceOffset), (const IrAction*)GET_FLASH_PTR(event->irActionOffset));
			}
			else if (event->type == EVENT_ACTIVITY) {
				selectActivity((const Activity*)GET_FLASH_PTR(event->activityOffset));
			}
			else if (event->type == EVENT_HOME) {
				selectActivity(homeActivity);
			}
			else if (event->type == EVENT_NEXTPAGE) {
				selectTouchPage(touchPage + 1);
			}
			else if (event->type == EVENT_PREVPAGE) {
				selectTouchPage(touchPage - 1);
			}
			else if (event->type == EVENT_DOWNLOAD) {
				if (!deviceAreAllOnDefault()) {
					turnOffAllDevices();
				}
				cpuFlashDownload();
				remoteInit();
				forceActivity(homeActivity);
			}
			else if (event->type == EVENT_POWEROFF) {
				if (!deviceAreAllOnDefault()) {
					turnOffAllDevices();
					forceActivity(homeActivity);
				}
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

	tpmInit();
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
	spiFlashInit();
	accelInit();

	if (FLASH_DATA_HEADER->watermark != FLASH_DATA_WATERMARK) {
		cpuFlashDownload();
	}

	mainLoop();

	//spiFlashTest();
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

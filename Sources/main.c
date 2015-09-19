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
#include "ports.h"
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
#define SLEEP_TIMEOUT	500

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

static volatile uint8_t periodicTimerIrqCount = 0;

static void periodicTimerIrqHandler()
{
	periodicTimerIrqCount++;
}

static void periodicTimerInit()
{
	// LPTMR version:
	SIM_SCGC5 |= SIM_SCGC5_LPTMR_MASK;

	interruptRegisterLPTMRIRQHandler(periodicTimerIrqHandler);
	NVIC_EnableIRQ(LPTimer_IRQn);
}

static void periodicTimerStart()
{
	LPTMR0_CSR = 0;												// Ensure timer is stopped and counter cleared
	LPTMR0_PSR = LPTMR_PSR_PCS(1) | LPTMR_PSR_PRESCALE(0);		// Counter frequency is 500Hz (1kHz LPO divided by 2)
	LPTMR0_CMR = 5;												// 100Hz interrupt
	LPTMR0_CSR = LPTMR_CSR_TIE_MASK | LPTMR_CSR_TEN_MASK;		// Enable with interrupt.
}

static void periodicTimerStop()
{
	LPTMR0_CSR = 0;												// Ensure timer is stopped and counter cleared
}

static uint32_t sleepCounter = 0;
static int isAsleep = 0;

static void sleep()
{
	isAsleep = 1;
	tftSetBacklight(0);
	tftSleep();
	periodicTimerStop();
}

static void wakeUp()
{
	if (isAsleep) {
		tftWake();
		tftSetBacklight(1);
		periodicTimerStart();
		isAsleep = 0;
	}

	sleepCounter = 0;
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

void idle()
{
	// Enter Very Low Power Stop mode
	SMC_PMCTRL &= ~SMC_PMCTRL_STOPM_MASK;
	SMC_PMCTRL |= SMC_PMCTRL_STOPM(2);
	/*wait for write to complete to SMC before stopping core */
	volatile uint32_t dummyread = SMC_PMCTRL;
	dummyread++;
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	__asm("wfi");

	SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

	// Switch PLL from PBE mode to PEE
	uint32_t exitClockMode = (MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT;

	if (exitClockMode != 3) {
		for (int i = 0 ; i < 2000 ; i++) {
			if (MCG_S & MCG_S_LOCK0_MASK) break; // jump out early if LOCK sets before loop finishes
		}

		MCG_C1 &= ~MCG_C1_CLKS_MASK; // clear CLKS to switch CLKS mux to select PLL as MCG_OUT

		// Wait for clock status bits to update
		for (int i = 0 ; i < 2000 ; i++) {
			if (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) == 0x3) break; // jump out early if CLKST = 3 before loop finishes
		}
	}

	NVIC_EnableIRQ(LPTimer_IRQn);
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

	periodicTimerInit();
	periodicTimerStart();

	keyMatrixClearInterrupt();
	touchScreenClearInterrupt();

	while (1) {
		idle();

		const Event* event = NULL;

		rendererNewDrawList();

		if (touchScreenCheckInterrupt()) {
			Point touch;
			if (touchScreenGetCoordinates(&touch)) {
				if (tftGetBacklight()) {
					touchbuttonsProcessTouch(&touch);
				}
				wakeUp();
			}
			touchScreenClearInterrupt();
		}

		if (keyMatrixCheckInterrupt()) {
			buttonsPollState();
			keyMatrixClearInterrupt();
		}

		if (accelCheckTransientInterrupt()) {
			accelClearInterrupts();
			wakeUp();
		}

		if (periodicTimerIrqCount) {
			buttonsPollState();
			touchButtonsUpdate(&event);
			periodicTimerIrqCount = 0;

			sleepCounter++;
			if (sleepCounter > SLEEP_TIMEOUT) {
				sleep();
			}
		}

		buttonsUpdate(&event);
		touchbuttonsRender();

		if (event) {
			wakeUp();
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

// On header: PTC0, PTC1, PTC2, PTB1, PTB2, PTB3, PTB19
// Others: PTB16, PTB17, PTC3, PTE29, PTE31

static const PortConfig unusedPortBPins =
{
	PORTB_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(0),
	4,
	{ 1, 2, 3, 16, 17, 19 }
};

static const PortConfig unusedPortCPins =
{
	PORTC_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(0),
	4,
	{ 8, 9, 10, 11 }
};

static const PortConfig unusedPortEPins =
{
	PORTE_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(0),
	2,
	{ 30, 31 }
};

static void tieDownUnusedPins()
{
	portInitialise(&unusedPortBPins);
	portInitialise(&unusedPortCPins);
	portInitialise(&unusedPortEPins);
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
	//accelInit();

	if (FLASH_DATA_HEADER->watermark != FLASH_DATA_WATERMARK) {
		cpuFlashDownload();
	}

	tieDownUnusedPins();
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

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
#include "capelectrode.h"
#include "slidergesture.h"
#include "debugutils.h"

//------------------------------------------
// Build configuration control
//------------------------------------------

//#define TIME_FRAME_RATE		// Run a short frame rate test on start
//#define DISABLE_KEYPAD		// Turns off keypad setup via I2C and capacitative slider reading
//#define ENABLE_TIMER_PIN		// Enables a PWM output on pin A13 to validate timing

#define SLEEP_TIMEOUT		500		// Time until backlight turns off when idle, in hundredths of a second
#define SLEEP_TIMEOUT_LONG	1000	// Time until backlight turns off when idle after touching screen, in hundredths of a second

//------------------------------------------

#define ACTIVE_LEVEL_AWAKE		0
#define ACTIVE_LEVEL_SLEEP		1
#define ACTIVE_LEVEL_DEEPSLEEP	2

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))


static int touchPage = 0;
static const Activity* currentActivity = NULL;

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

static void periodicTimerStartSlow()
{
	LPTMR0_CSR = 0;												// Ensure timer is stopped and counter cleared
	LPTMR0_PSR = LPTMR_PSR_PCS(1) | LPTMR_PSR_PRESCALE(8);		// Counter frequency is 1.95Hz (1kHz LPO divided by 512)
	LPTMR0_CMR = 20;											// Interrupt every 10 seconds or so
	LPTMR0_CSR = LPTMR_CSR_TIE_MASK | LPTMR_CSR_TEN_MASK;		// Enable with interrupt.
}

static void periodicTimerStop()
{
	LPTMR0_CSR = 0;												// Ensure timer is stopped and counter cleared
	periodicTimerIrqCount = 0;
}

static uint32_t sleepCounter = SLEEP_TIMEOUT;
static int activeLevel = ACTIVE_LEVEL_AWAKE;

static void sleep()
{
	activeLevel = ACTIVE_LEVEL_SLEEP;
	tftSetBacklight(0);
	tftSleep();
	periodicTimerStop();
	periodicTimerStartSlow();
}

static void deepSleep()
{
	activeLevel = ACTIVE_LEVEL_DEEPSLEEP;
	touchScreenDisconnect();
	spiPinsDisconnect();
	tftPowerOff();
	capElectrodeSleep();
	periodicTimerStop();
}

static void sleepNow()
{
	buttonsClearState();
	sleepCounter = 0;
	sleep();
}

static void wakeUp(uint32_t wake_time_hs)
{
	if (activeLevel != ACTIVE_LEVEL_AWAKE) {
		if (activeLevel == ACTIVE_LEVEL_DEEPSLEEP) {
			capElectrodeWake();
			tftPowerOn();
			touchScreenConnect();
			spiPinsConnect();
			rendererClearScreen();
			touchbuttonsRedraw();
		}
		else {
			tftWake();
		}

		tftSetBacklight(1);
		periodicTimerStop();
		periodicTimerStart();
		activeLevel = ACTIVE_LEVEL_AWAKE;
	}

	if (sleepCounter < wake_time_hs) {
		sleepCounter = wake_time_hs;
	}
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
		sliderGestureSetActiveMapping((const GestureMapping*) GET_FLASH_PTR(activity->gestureMappingOffset), activity->gestureMappingCount);

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

	debugUtilsInit();

	const Activity* homeActivity = remoteInit();
	selectActivity(homeActivity);

#ifdef TIME_FRAME_RATE
	volatile int frames = 0;
	sysTickEventInMs(1000);
	while (!sysTickCheckEvent()) {
		touchbuttonsRedraw();
		rendererNewDrawList();
		touchbuttonsRender();
		rendererRenderDrawList();
		frames++;
	}

	debugSetOverlayHex(0, frames);
	debugRenderOverlays();

	sysTickEventInMs(2000);
	while (!sysTickCheckEvent());
	touchbuttonsRedraw();
#endif

	periodicTimerInit();
	periodicTimerStart();

	keyMatrixClearInterrupt();
	touchScreenClearInterrupt();

	uint32_t frameCounter = 0;

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
				wakeUp(SLEEP_TIMEOUT_LONG);
			}
			touchScreenClearInterrupt();
		}

		if (keyMatrixCheckInterrupt()) {
			int changed = buttonsPollState();
			keyMatrixClearInterrupt();
			if (changed) {
				wakeUp(SLEEP_TIMEOUT);
			}
		}

		if (accelCheckTransientInterrupt()) {
			accelClearInterrupts();
			wakeUp(SLEEP_TIMEOUT);
		}

		if (periodicTimerIrqCount) {
			if (activeLevel == ACTIVE_LEVEL_SLEEP) {
				deepSleep();
			}
			else {
				frameCounter += periodicTimerIrqCount;

//				debugSetOverlayHex(0, periodicTimerIrqCount);
//				debugSetOverlayHex(1, frameCounter);

				touchButtonsUpdate(&event);
				periodicTimerIrqCount = 0;

#ifndef DISABLE_KEYPAD
				if (sliderGestureUpdate(frameCounter, &event) != EVENT_NONE) {
					wakeUp(SLEEP_TIMEOUT_LONG);
				}
#endif

				sleepCounter--;
				if (sleepCounter == 0) {
					sleep();
				}
			}
		}

		buttonsUpdate(&event);
		touchbuttonsRender();

		if (event) {
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
					sleepNow();
				}
			}
		}
		else {
			rendererRenderDrawList();
		}

		debugRenderOverlays();
	}
}

static const PortConfig unusedPortAPins =
{
	PORTA_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(0),
#ifdef ENABLE_TIMER_PIN
	3,
	{ 1, 2, 5 }
#else
	4,
	{ 1, 2, 5, 13 }
#endif
};

static const PortConfig unusedPortBPins =
{
	PORTB_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(0),
	5,
	{ 1, 2, 16, 17, 19 }
};

static const PortConfig unusedPortCPins =
{
	PORTC_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(0),
	4,
	{ 8, 9, 10, 11 }
};

static void tieDownUnusedPins()
{
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK|SIM_SCGC5_PORTB_MASK|SIM_SCGC5_PORTC_MASK;
	portInitialise(&unusedPortAPins);
	portInitialise(&unusedPortBPins);
	portInitialise(&unusedPortCPins);
}

static const PortConfig timerPin =
{
	PORTA_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(3),	// TPM 1 CH 1
	1,
	{ 13 }
};

#ifdef ENABLE_TIMER_PIN
static void startTimerPin()
{
	portInitialise(&timerPin);
	tpmStartPwm(1, 23, 1, 12);
}
#endif

void main(void)
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

#ifndef DISABLE_KEYPAD
	keyMatrixInit();
	sliderGestureInit();
#endif

	accelInit();

	if (FLASH_DATA_HEADER->watermark != FLASH_DATA_WATERMARK) {
		cpuFlashDownload();
	}

	tieDownUnusedPins();

#ifdef ENABLE_TIMER_PIN
	startTimerPin();
#endif

	mainLoop();
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

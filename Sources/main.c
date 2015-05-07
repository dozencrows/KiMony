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
			systickDelayMs(100);
		}
	}
}

int main(void)
{
	SystemCoreClockUpdate();

	systickInit();
	systickSetClockRate(SystemCoreClock);

	i2cInit();
	spiInit();

	keyMatrixInit();
	touchScreenInit();
	tftInit();
	tftSetBacklight(1);

	irTest();
	testKeyMatrix();
	touchScreenTest();

	int frames = 0;
	systickEventInMs(1000);
	while (!systickCheckEvent()) {
		drawTestRectDma();
		frames++;
	}

	printf("DMA %d\n", frames);
	waitForButton();
	drawTestRect_PEInline_FGPIO(0);

	frames = 0;
	systickEventInMs(1000);
	while (!systickCheckEvent()) {
		drawTestRect_PEInline_FGPIO(0x1ff8);
		frames++;
	}

	printf("FGPIO %d\n", frames);
	waitForButton();
	drawTestRect_PEInline_FGPIO(0);

	frames = 0;
	systickEventInMs(1000);
	while (!systickCheckEvent()) {
		drawTestRect_PEInline_BME(0x1ff8);
		frames++;
	}

	printf("BME %d\n", frames);
	waitForButton();
	drawTestRect_PEInline_FGPIO(0);

	frames = 0;
	systickEventInMs(1000);
	while (!systickCheckEvent()) {
		drawTestRect_PEInline_SRAM(0x1ff8);
		frames++;
	}

	printf("FGPIO SRAM %d\n", frames);
	waitForButton();
	drawTestRect_PEInline_FGPIO(0);

	frames = 0;
	systickEventInMs(1000);
	while (!systickCheckEvent()) {
		drawTestRect_PEInline_SRAM_PDOR(0x1ff8);
		frames++;
	}

	printf("FGPIO SRAM PDOR %d\n", frames);
	waitForButton();
	drawTestRect_PEInline_FGPIO(0);

	frames = 0;
	systickEventInMs(1000);
	while (!systickCheckEvent()) {
		drawTestRect_PEInline_SRAM_PDOR_BufferFill(0x1ff8);
		frames++;
	}

	printf("FGPIO SRAM PDOR BufferFill %d\n", frames);
	waitForButton();
	drawTestRect_PEInline_FGPIO(0);

	frames = 0;
	systickEventInMs(1000);
	while (!systickCheckEvent()) {
		drawTestRect_PEInline_WROnly(0x1ff8);
		frames++;
	}

	printf("WROnly %d\n", frames);
	waitForButton();
	drawTestRect_PEInline_FGPIO(0);

	frames = 0;
	systickEventInMs(1000);
	while (!systickCheckEvent()) {
		drawTestImage(0, 0);
		drawTestImage(120, 0);
		drawTestImage(0, 160);
		drawTestImage(120, 160);
		frames++;
	}

	printf("FGPIO Flash %d\n", frames);

	for(;;) {
	}

    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

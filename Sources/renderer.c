/*
 * renderer.c
 *
 *  Created on: 10 May 2015
 *      Author: ntuckett
 */
#include "renderer.h"

#include "lcd.h"


uint16_t pixelBuffer[240];

void rendererTest()
{
	for (int i = 0; i < 240; i++) {
		pixelBuffer[i] = 0x1ff8;
	}

	tftStartBlit(0, 0, 240, 320);

	for (int y = 0; y < 320; y++) {
		tftBlit(pixelBuffer, 240);
	}

	tftEndBlit();
}


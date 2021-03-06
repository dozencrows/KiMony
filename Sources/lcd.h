//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * lcd.h
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#ifndef LCD_H_
#define LCD_H_
#include <stdint.h>
#include <stddef.h>

extern void tftInit();
extern void tftSetBacklight(int status);
extern int tftGetBacklight();
extern void tftSleep();
extern void tftWake();
extern void tftPowerOff();
extern void tftPowerOn();
extern void tftStartBlit(int x, int y, int width, int height);
extern void tftBlit(uint16_t* buffer, size_t pixels);
extern void tftClear(size_t pixels);
extern void tftEndBlit();

extern void drawTestRect_PEInline_FGPIO(uint16_t colour);
extern void drawTestRect_PEInline_BME(uint16_t colour);
extern void drawTestRect_PEInline_WROnly(uint16_t colour);
extern void drawTestRect_PEInline_SRAM(uint16_t colour);
extern void drawTestRect_PEInline_SRAM_PDOR(uint16_t colour);
extern void drawTestRect_PEInline_SRAM_PDOR_BufferFill(uint16_t colour);
extern void drawTestImage(uint16_t x0, uint16_t y0);
extern void drawTestRectDma();

#endif /* LCD_H_ */

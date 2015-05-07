/*
 * lcd.h
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#ifndef LCD_H_
#define LCD_H_
#include <stdint.h>

extern void tftInit();
extern void tftSetBacklight(int status);

extern void drawTestRect_PEInline_FGPIO(uint16_t colour);
extern void drawTestRect_PEInline_BME(uint16_t colour);
extern void drawTestRect_PEInline_WROnly(uint16_t colour);
extern void drawTestRect_PEInline_SRAM(uint16_t colour);
extern void drawTestRect_PEInline_SRAM_PDOR(uint16_t colour);
extern void drawTestRect_PEInline_SRAM_PDOR_BufferFill(uint16_t colour);
extern void drawTestImage(uint16_t x0, uint16_t y0);
extern void drawTestRectDma();

#endif /* LCD_H_ */

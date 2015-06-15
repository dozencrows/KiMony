//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * renderer.h
 *
 *  Created on: 10 May 2015
 *      Author: ntuckett
 */

#ifndef RENDERER_H_
#define RENDERER_H_
#include <stdint.h>

typedef struct _Font Font;
typedef struct _Glyph Glyph;
typedef struct _Image Image;

#define SCREEN_WIDTH	240
#define SCREEN_HEIGHT	320

extern void rendererInit();
extern void rendererClearScreen();
extern void rendererNewDrawList();
extern void rendererDrawVLine(uint16_t x, uint16_t y, uint16_t length, uint16_t colour);
extern void rendererDrawHLine(uint16_t x, uint16_t y, uint16_t length, uint16_t colour);
extern void rendererDrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t colour);
extern void rendererDrawGlyph(const Glyph* glyph, uint16_t x, uint16_t y, uint16_t colour);
extern void rendererDrawChar(char c, uint16_t x, uint16_t y, const Font* font, uint16_t colour);
extern void rendererDrawString(const char* s, uint16_t x, uint16_t y, const Font* font, uint16_t colour);
extern void rendererDrawImage(const Image* i, uint16_t x, uint16_t y);
extern void rendererRenderDrawList();
extern void rendererGetStringBounds(const char* s, const Font* font, uint16_t* width, uint16_t* height);

extern void rendererTest();

#endif /* RENDERER_H_ */

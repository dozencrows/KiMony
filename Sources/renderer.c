/*
 * renderer.c
 *
 *  Created on: 10 May 2015
 *      Author: ntuckett
 */
#include "renderer.h"
#include <string.h>
#include "lcd.h"
#include "mathutil.h"
#include "fontdata.h"

#define DRAWLIST_BUFFER_SIZE	2048

#define DLE_FLAG_ACTIVE_MASK	0x01
#define DLE_FLAG_DRAWN_MASK		0x02
#define DLE_TYPE_MASK			0xf0

#define DLE_TYPE_VLINE	0x10
#define DLE_TYPE_HLINE	0x20
#define DLE_TYPE_RECT	0x30
#define DLE_TYPE_TXTCH	0x40

typedef struct _DrawListEntry {
	uint8_t		flags;
	uint8_t		size;
} DrawListEntry;

typedef struct _Line {
	DrawListEntry	dle;
	uint16_t		x;
	uint16_t		y;
	uint16_t		length;
	uint16_t		colour;
} Line;

typedef struct _Rect {
	DrawListEntry	dle;
	uint16_t		x;
	uint16_t		y;
	uint16_t		width;
	uint16_t		height;
	uint16_t		colour;
} Rect;

typedef struct _TextChar {
	DrawListEntry	dle;
	uint16_t		x;
	uint16_t		y;
	const Glyph*	glyph;
	uint16_t		colour;
} TextChar;

uint8_t		drawListBuffer[DRAWLIST_BUFFER_SIZE];
size_t		drawListEnd = 0;
uint16_t	drawListMinX = SCREEN_WIDTH;
uint16_t	drawListMinY = SCREEN_HEIGHT;
uint16_t	drawListMaxX = 0;
uint16_t	drawListMaxY = 0;
uint16_t 	pixelBuffer[SCREEN_WIDTH];

static DrawListEntry* allocDrawListEntry(size_t bytes)
{
	if (drawListEnd + bytes < DRAWLIST_BUFFER_SIZE) {
		DrawListEntry* dle = (DrawListEntry*) (drawListBuffer + drawListEnd);
		dle->size = bytes;
		drawListEnd += bytes;
		return dle;
	}
	else {
		return NULL;
	}
}

static void updateDrawListBounds(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	if (x0 > x1) {
		SWAPNUM(x0, x1);
	}

	if (y0 > y1) {
		SWAPNUM(y0, y1);
	}

	if (x0 < drawListMinX) {
		drawListMinX = MAX(x0, 0);
	}

	if (x1 > drawListMaxX) {
		drawListMaxX = MIN(x1, SCREEN_WIDTH);
	}

	if (y0 < drawListMinY) {
		drawListMinY = MAX(y0, 0);
	}

	if (y1 > drawListMaxY) {
		drawListMaxY = MIN(y1, SCREEN_HEIGHT);
	}
}

static void renderScanLine(uint16_t y)
{
	size_t drawListIndex = 0;

	memset(pixelBuffer, 0, (drawListMaxX - drawListMinX)*2);

	while (drawListIndex < drawListEnd) {
		DrawListEntry* dle = (DrawListEntry*) (drawListBuffer + drawListIndex);
		if (!(dle->flags & DLE_FLAG_DRAWN_MASK)) {
			if (!(dle->flags & DLE_FLAG_ACTIVE_MASK)) {
				switch(dle->flags & DLE_TYPE_MASK) {
					case DLE_TYPE_VLINE:
					case DLE_TYPE_HLINE: {
						Line* line = (Line*)dle;
						if (y == line->y) {
							dle->flags |= DLE_FLAG_ACTIVE_MASK;
						}
						break;
					}
					case DLE_TYPE_RECT: {
						Rect* rect = (Rect*)dle;
						if (y == rect->y) {
							dle->flags |= DLE_FLAG_ACTIVE_MASK;
						}
						break;
					}
					case DLE_TYPE_TXTCH: {
						TextChar* textChar = (TextChar*)dle;
						if (y == textChar->y) {
							dle->flags |= DLE_FLAG_ACTIVE_MASK;
						}
					}
					default: {
						break;
					}
				}
			}

			if (dle->flags & DLE_FLAG_ACTIVE_MASK) {
				switch(dle->flags & DLE_TYPE_MASK) {
					case DLE_TYPE_VLINE: {
						Line* vLine = (Line*)dle;
						if (y == vLine->y + vLine->length) {
							dle->flags &= ~DLE_FLAG_ACTIVE_MASK;
							dle->flags |= DLE_FLAG_DRAWN_MASK;
						}
						else {
							pixelBuffer[vLine->x - drawListMinX] = vLine->colour;
						}
						break;
					}
					case DLE_TYPE_HLINE: {
						Line* hLine = (Line*)dle;
						for (uint16_t x = 0; x < hLine->length; x++) {
							pixelBuffer[hLine->x - drawListMinX + x] = hLine->colour;
						}
						dle->flags &= ~DLE_FLAG_ACTIVE_MASK;
						dle->flags |= DLE_FLAG_DRAWN_MASK;
						break;
					}
					case DLE_TYPE_RECT: {
						Rect* rect = (Rect*)dle;
						if (y == rect->y + rect->height) {
							dle->flags &= ~DLE_FLAG_ACTIVE_MASK;
							dle->flags |= DLE_FLAG_DRAWN_MASK;
						}
						else {
							for (uint16_t x = 0; x < rect->width; x++) {
								pixelBuffer[rect->x - drawListMinX + x] = rect->colour;
							}
						}
						break;
					}
					case DLE_TYPE_TXTCH: {
						TextChar* textChar = (TextChar*)dle;
						if (y == textChar->y + textChar->glyph->height) {
							dle->flags &= ~DLE_FLAG_ACTIVE_MASK;
							dle->flags |= DLE_FLAG_DRAWN_MASK;
						}
						else {
							const uint8_t* char_row = textChar->glyph->data + (y - textChar->y) * textChar->glyph->width;
							for (uint16_t x = 0; x < textChar->glyph->width; x++) {
								if (!char_row[x]) {
									pixelBuffer[textChar->x - drawListMinX + x] = textChar->colour;
								}
							}
						}
					}
					default: {
						break;
					}
				}
			}
		}

		drawListIndex += dle->size;
	}
}

void rendererInit()
{
	rendererClearScreen();
	rendererNewDrawList();
}

void rendererClearScreen()
{
	tftStartBlit(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	tftClear(SCREEN_WIDTH * SCREEN_HEIGHT);
	tftEndBlit();
}

void rendererNewDrawList()
{
	drawListEnd = 0;
	drawListMinX = SCREEN_WIDTH;
	drawListMinY = SCREEN_HEIGHT;
	drawListMaxX = 0;
	drawListMaxY = 0;
}

void rendererDrawVLine(uint16_t x, uint16_t y, uint16_t length, uint16_t colour)
{
	Line* vLine = (Line*) allocDrawListEntry(sizeof(Line));

	if (vLine) {
		vLine->dle.flags 	= DLE_TYPE_VLINE;
		vLine->x			= x;
		vLine->y			= y;
		vLine->length		= length;
		vLine->colour		= colour;

		updateDrawListBounds(x, y, x + 1, y + length);
	}
}

void rendererDrawHLine(uint16_t x, uint16_t y, uint16_t length, uint16_t colour)
{
	Line* vLine = (Line*) allocDrawListEntry(sizeof(Line));

	if (vLine) {
		vLine->dle.flags 	= DLE_TYPE_HLINE;
		vLine->x			= x;
		vLine->y			= y;
		vLine->length		= length;
		vLine->colour		= colour;

		updateDrawListBounds(x, y, x + length, y + 1);
	}
}

void rendererDrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t colour)
{
	Rect* rect = (Rect*) allocDrawListEntry(sizeof(Rect));

	if (rect) {
		rect->dle.flags 	= DLE_TYPE_RECT;
		rect->x				= x;
		rect->y				= y;
		rect->width			= width;
		rect->height		= height;
		rect->colour		= colour;

		updateDrawListBounds(x, y, x + width, y + height);
	}
}

static const Glyph* findGlyph(char c, const Font* font)
{
	const Glyph* glyph = NULL;
	if (c >= font->chars[0].code && c <= font->chars[font->length - 1].code) {

		for (int i = 0; i < font->length; i++) {
			if (font->chars[i].code == c) {
				glyph = font->chars[i].image;
				break;
			}
			if (font->chars[i].code > c) {
				break;
			}
		}
	}

	return glyph;
}

void rendererDrawGlyph(const Glyph* glyph, uint16_t x, uint16_t y, uint16_t colour)
{
	TextChar* textChar = (TextChar*) allocDrawListEntry(sizeof(TextChar));

	if (textChar) {
		textChar->dle.flags	= DLE_TYPE_TXTCH;
		textChar->x			= x;
		textChar->y			= y;
		textChar->colour	= colour;
		textChar->glyph		= glyph;

		updateDrawListBounds(x, y, x + glyph->width, y + glyph->height);
	}
}

void rendererDrawChar(char c, uint16_t x, uint16_t y, const Font* font, uint16_t colour)
{
	const Glyph* glyph = findGlyph(c, font);

	if (glyph) {
		rendererDrawGlyph(glyph, x, y, colour);
	}
}

void rendererDrawString(char* s, uint16_t x, uint16_t y, const Font* font, uint16_t colour)
{
	char c;

	while ((c = *s++)) {
		const Glyph* glyph = findGlyph(c, font);

		if (!glyph) {
			glyph = findGlyph('*', font);
		}

		if (glyph) {
			rendererDrawGlyph(glyph, x, y, colour);
			x += glyph->width;
		}
	}
}

void rendererRenderDrawList() {
	if (drawListMaxX > drawListMinX) {
		tftStartBlit(drawListMinX, drawListMinY, drawListMaxX - drawListMinX, drawListMaxY - drawListMinY);

		for(uint16_t y = drawListMinY; y < drawListMaxY; y++) {
			renderScanLine(y);
			tftBlit(pixelBuffer, drawListMaxX - drawListMinX);
		}

		tftEndBlit();
	}
}

void rendererTest()
{
	rendererNewDrawList();
	rendererDrawVLine(0, 32, 256, 0xffff);
	rendererDrawVLine(119, 32, 256, 0xffff);
	rendererDrawHLine(0, 32, 120, 0xffff);
	rendererDrawHLine(0, 287, 120, 0xffff);
	rendererDrawRect(1, 33, 118, 254, 0xfa00);
	rendererDrawChar('A', 10, 40, &KiMony, 0xffff);
	rendererDrawChar('c', 21, 40, &KiMony, 0x1ff8);
	rendererDrawString("Hello", 10, 52, &KiMony, 0xf81f);
	rendererRenderDrawList();
}


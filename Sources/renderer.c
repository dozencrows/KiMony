//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * renderer.c
 *
 *  Created on: 10 May 2015
 *      Author: ntuckett
 */
#include "renderer.h"
#include <string.h>
#include <stdio.h>
#include "assertbrk.h"
#include "lcd.h"
#include "flash.h"
#include "systick.h"
#include "mathutil.h"
#include "fontdata.h"
#include "image.h"
#include "profiler.h"

#define DRAWLIST_BUFFER_SIZE	4608
#define DLE_FLAG_ACTIVE_MASK	0x01
#define DLE_FLAG_DRAWN_MASK		0x02
#define DLE_TYPE_MASK			0xf0

#define DLE_TYPE_VLINE	0x10
#define DLE_TYPE_HLINE	0x20
#define DLE_TYPE_RECT	0x30
#define DLE_TYPE_TXTCH	0x40
#define DLE_TYPE_IMAGE	0x50

typedef struct _DrawListEntry {
	struct _DrawListEntry*	next;
	uint8_t					flags;
	uint8_t					size;
	uint16_t				y;
} DrawListEntry;

typedef struct _LineDLE {
	DrawListEntry	dle;
	uint16_t		x;
	uint16_t		length;
	uint16_t		colour;
} LineDLE;

typedef struct _RectDLE {
	DrawListEntry	dle;
	uint16_t		x;
	uint16_t		width;
	uint16_t		height;
	uint16_t		colour;
} RectDLE;

typedef struct _GlyphDLE {
	DrawListEntry	dle;
	uint16_t		x;
	uint16_t		colour;
	const Glyph*	glyph;
	const uint8_t*	data;
} GlyphDLE;

typedef struct _ImageDLE {
	DrawListEntry	dle;
	uint16_t		x;
	const Image*	image;
	const uint16_t*	palette;
	const uint8_t*	pixels;
} ImageDLE;

uint8_t		drawListBuffer[DRAWLIST_BUFFER_SIZE];
DrawListEntry* 	activeDLEs = NULL;
DrawListEntry** activeDLETail = &activeDLEs;
DrawListEntry* 	pendingDLEs = NULL;
DrawListEntry** pendingDLETail = &pendingDLEs;

size_t		drawListEnd = 0;
uint16_t	drawListMinX = SCREEN_WIDTH;
uint16_t	drawListMinY = SCREEN_HEIGHT;
uint16_t	drawListMaxX = 0;
uint16_t	drawListMaxY = 0;
uint16_t 	pixelBuffer[SCREEN_WIDTH];
uint8_t		rowMinX[SCREEN_HEIGHT];
uint8_t		rowMaxX[SCREEN_HEIGHT];

static DrawListEntry* allocDrawListEntry(size_t bytes)
{
	if (drawListEnd + bytes < DRAWLIST_BUFFER_SIZE) {
		DrawListEntry* dle = (DrawListEntry*) (drawListBuffer + drawListEnd);
		dle->size = bytes;
		dle->next = NULL;
		drawListEnd += bytes;
		return dle;
	}
	else {
		return NULL;
	}
}

void insertPendingDrawListEntry(DrawListEntry* dle)
{
	if (pendingDLEs == NULL) {
		*pendingDLETail = dle;
		pendingDLETail = &dle->next;
	}
	else {
		DrawListEntry* candidateDLE = pendingDLEs;
		DrawListEntry** candidateDLELink = &pendingDLEs;

		while (candidateDLE && dle->y >= candidateDLE->y) {
			candidateDLELink = &candidateDLE->next;
			candidateDLE = candidateDLE->next;
		}

		*candidateDLELink = dle;
		dle->next = candidateDLE;

		if (!candidateDLE) {
			pendingDLETail = &dle->next;
		}
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

	y0 = MAX(y0, 0);
	y1 = MIN(y1, SCREEN_HEIGHT);
	x0 = MAX(x0, 0);
	x1 = MIN(x1, SCREEN_WIDTH);

	if (x0 < drawListMinX) {
		drawListMinX = x0;
	}

	if (x1 > drawListMaxX) {
		drawListMaxX = x1;
	}

	if (y0 < drawListMinY) {
		drawListMinY = y0;
	}

	if (y1 > drawListMaxY) {
		drawListMaxY = y1;
	}

	for (int y = y0; y < y1; y++) {
		if (x0 < rowMinX[y]) {
			rowMinX[y] = x0;
		}

		if (x1 > rowMaxX[y]) {
			rowMaxX[y] = x1;
		}
	}
}

static void renderScanLine(uint16_t y)
{
	//PROFILE_ENTER(scanline);

	size_t undrawnCount = 0;

	DrawListEntry** lastDle = &pendingDLEs;
	DrawListEntry* dle = pendingDLEs;

	//PROFILE_ENTER(activationCheck);
	while(dle && y == dle->y) {
		DrawListEntry* nextDle = dle->next;

		if (y == dle->y) {
			*lastDle = nextDle;
			dle->next = NULL;
			*activeDLETail = dle;
			activeDLETail = &dle->next;
		}

		if (dle->next == nextDle) {
			lastDle = &dle->next;
		}
		dle = nextDle;
	}
	//PROFILE_EXIT(activationCheck);

	lastDle = &activeDLEs;
	dle = activeDLEs;

	uint16_t minX = drawListMinX;

	int fillWidth = rowMinX[y] - drawListMinX;
	uint16_t* fillPtr = pixelBuffer;

	if (fillWidth >= 2) {
		uint32_t  fillDuo = 0;
		uint32_t* fillPtrDuo = (uint32_t*)fillPtr;
		while(fillWidth > 3) {
			*(fillPtrDuo++) = fillDuo;
			*(fillPtrDuo++) = fillDuo;
			fillWidth -= 4;
		}
		while(fillWidth > 1) {
			*(fillPtrDuo++) = fillDuo;
			fillWidth -= 2;
		}
		fillPtr = (uint16_t*)fillPtrDuo;
	}

	while (fillWidth > 0) {
		*fillPtr++ = 0;
		fillWidth--;
	}

	//PROFILE_ENTER(primitives);
	while(dle) {
		DrawListEntry* nextDle = dle->next;

		switch(dle->flags & DLE_TYPE_MASK) {
			case DLE_TYPE_VLINE: {
				//PROFILE_ENTER(vline);
				LineDLE* vLine = (LineDLE*)dle;
				if (y == dle->y + vLine->length) {
					*lastDle = nextDle;
					dle->next = dle;
				}
				else {
					pixelBuffer[vLine->x - minX] = vLine->colour;
				}
				//PROFILE_EXIT(vline);
				break;
			}
			case DLE_TYPE_HLINE: {
				//PROFILE_ENTER(hline);
				LineDLE* hLine = (LineDLE*)dle;
				for (uint16_t x = 0; x < hLine->length; x++) {
					pixelBuffer[hLine->x - minX + x] = hLine->colour;
				}
				*lastDle = nextDle;
				dle->next = dle;
				//PROFILE_EXIT(hline);
				break;
			}
			case DLE_TYPE_RECT: {
				//PROFILE_ENTER(rect);
				RectDLE* rect = (RectDLE*)dle;
				if (y == dle->y + rect->height) {
					*lastDle = nextDle;
					dle->next = dle;
				}
				else {
					int width = rect->width;
					uint16_t* pixPtr = pixelBuffer + rect->x - minX;
					if (((uint32_t)pixPtr) & 3) {
						*pixPtr++ = rect->colour;
						width--;
					}

					if (width >= 2) {
						uint32_t  pixDuo = (uint32_t)rect->colour | ((uint32_t)rect->colour << 16);
						uint32_t* pixPtrDuo = (uint32_t*)pixPtr;
						while(width > 3) {
							*(pixPtrDuo++) = pixDuo;
							*(pixPtrDuo++) = pixDuo;
							width -= 4;
						}
						while(width > 1) {
							*(pixPtrDuo++) = pixDuo;
							width -= 2;
						}
						pixPtr = (uint16_t*)pixPtrDuo;
					}

					while (width--) {
						*pixPtr++ = rect->colour;
					}
				}
				//PROFILE_EXIT(rect);
				break;
			}
			case DLE_TYPE_TXTCH: {
				//PROFILE_ENTER(text);
				GlyphDLE* textChar = (GlyphDLE*)dle;
				if (y == dle->y + textChar->glyph->height) {
					*lastDle = nextDle;
					dle->next = dle;
				}
				else {
					const uint8_t* charData = textChar->data;
					uint16_t* pixPtr = pixelBuffer + textChar->x - minX;
					uint16_t width = textChar->glyph->width;
					while (width-- > 0) {
						if (!(*charData)) {
							*pixPtr = textChar->colour;
						}
						charData++;
						pixPtr++;
					}
					textChar->data = charData;
				}
				//PROFILE_EXIT(text);
				break;
			}
			case DLE_TYPE_IMAGE: {
				//PROFILE_ENTER(image);
				ImageDLE* image = (ImageDLE*)dle;
				if (y == dle->y + image->image->height) {
					*lastDle = nextDle;
					dle->next = dle;
				}
				else {
					const uint8_t* imagePix = image->pixels;
					const uint16_t* imagePal = image->palette;
					uint16_t* pixPtr = pixelBuffer + image->x - minX;
					uint16_t width = image->image->width;

					while (width-- > 0) {
						uint8_t pix = *imagePix++;
						if (pix) {
							*pixPtr++ = imagePal[pix];
						}
						else {
							pixPtr++;
						}
					}

					image->pixels = imagePix;
				}
				//PROFILE_EXIT(image);
				break;
			}
			default: {
				break;
			}
		}

		if (dle->next != dle) {
			lastDle = &dle->next;
		}
		else if (nextDle == NULL) {
			activeDLETail = lastDle;
		}

		dle = nextDle;
	}

	fillWidth = drawListMaxX - rowMaxX[y];
	fillPtr = pixelBuffer + rowMaxX[y] - drawListMinX;

	if (((uint32_t)fillPtr) & 3) {
		*fillPtr++ = 0;
		fillWidth--;
	}

	if (fillWidth >= 2) {
		uint32_t  fillDuo = 0;
		uint32_t* fillPtrDuo = (uint32_t*)fillPtr;
		while(fillWidth > 3) {
			*(fillPtrDuo++) = fillDuo;
			*(fillPtrDuo++) = fillDuo;
			fillWidth -= 4;
		}
		while(fillWidth > 1) {
			*(fillPtrDuo++) = fillDuo;
			fillWidth -= 2;
		}
		fillPtr = (uint16_t*)fillPtrDuo;
	}

	while (fillWidth > 0) {
		*fillPtr++ = 0;
		fillWidth--;
	}

	//PROFILE_EXIT(primitives);
	//PROFILE_EXIT(scanline);
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
	activeDLEs = NULL;
	activeDLETail = &activeDLEs;
	pendingDLEs	= NULL;
	pendingDLETail = &pendingDLEs;
	drawListEnd = 0;
	drawListMinX = SCREEN_WIDTH;
	drawListMinY = SCREEN_HEIGHT;
	drawListMaxX = 0;
	drawListMaxY = 0;

	memset(rowMinX, SCREEN_WIDTH, SCREEN_HEIGHT);
	memset(rowMaxX, 0, SCREEN_HEIGHT);
	PROFILE_BEGIN;
}

void rendererDrawVLine(uint16_t x, uint16_t y, uint16_t length, uint16_t colour)
{
	ASSERTBRK(length > 0);
	ASSERTBRK(x >= 0 && x < SCREEN_WIDTH);
	ASSERTBRK(y >= 0 && y < SCREEN_HEIGHT);
	ASSERTBRK(y + length <= SCREEN_HEIGHT);

	LineDLE* vLine = (LineDLE*) allocDrawListEntry(sizeof(LineDLE));

	if (vLine) {
		vLine->dle.flags 	= DLE_TYPE_VLINE;
		vLine->dle.y		= y;
		vLine->x			= x;
		vLine->length		= length;
		vLine->colour		= colour;

		insertPendingDrawListEntry(&vLine->dle);
		updateDrawListBounds(x, y, x + 1, y + length);
	}
}

#include <assert.h>

void rendererDrawHLine(uint16_t x, uint16_t y, uint16_t length, uint16_t colour)
{
	ASSERTBRK(length > 0);
	ASSERTBRK(x >= 0 && x < SCREEN_WIDTH);
	ASSERTBRK(y >= 0 && y < SCREEN_HEIGHT);
	ASSERTBRK(x + length <= SCREEN_WIDTH);

	LineDLE* vLine = (LineDLE*) allocDrawListEntry(sizeof(LineDLE));

	if (vLine) {
		vLine->dle.flags 	= DLE_TYPE_HLINE;
		vLine->dle.y		= y;
		vLine->x			= x;
		vLine->length		= length;
		vLine->colour		= colour;

		insertPendingDrawListEntry(&vLine->dle);
		updateDrawListBounds(x, y, x + length, y + 1);
	}
}

void rendererDrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t colour)
{
	ASSERTBRK(width > 0);
	ASSERTBRK(height > 0);
	ASSERTBRK(x >= 0 && x < SCREEN_WIDTH);
	ASSERTBRK(y >= 0 && y < SCREEN_HEIGHT);
	ASSERTBRK(x + width <= SCREEN_WIDTH);
	ASSERTBRK(y + height <= SCREEN_HEIGHT);

	RectDLE* rect = (RectDLE*) allocDrawListEntry(sizeof(RectDLE));

	if (rect) {
		rect->dle.flags 	= DLE_TYPE_RECT;
		rect->dle.y			= y;
		rect->x				= x;
		rect->width			= width;
		rect->height		= height;
		rect->colour		= colour;

		insertPendingDrawListEntry(&rect->dle);
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
	ASSERTBRK(x >= 0 && x < SCREEN_WIDTH);
	ASSERTBRK(y >= 0 && y < SCREEN_HEIGHT);

	GlyphDLE* textChar = (GlyphDLE*) allocDrawListEntry(sizeof(GlyphDLE));

	if (textChar) {
		textChar->dle.flags	= DLE_TYPE_TXTCH;
		textChar->dle.y		= y;
		textChar->x			= x;
		textChar->colour	= colour;
		textChar->glyph		= glyph;
		textChar->data		= glyph->data;

		insertPendingDrawListEntry(&textChar->dle);
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

void rendererDrawString(const char* s, uint16_t x, uint16_t y, const Font* font, uint16_t colour)
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

void rendererDrawImage(const Image* i, uint16_t x, uint16_t y)
{
	ASSERTBRK(i != NULL);
	ASSERTBRK(i->width > 0);
	ASSERTBRK(i->height > 0);
	ASSERTBRK(x >= 0 && x < SCREEN_WIDTH);
	ASSERTBRK(y >= 0 && y < SCREEN_HEIGHT);
	ASSERTBRK(x + i->width <= SCREEN_WIDTH);
	ASSERTBRK(y + i->height <= SCREEN_HEIGHT);

	ImageDLE* imageDle = (ImageDLE*) allocDrawListEntry(sizeof(ImageDLE));

	if (imageDle) {
		imageDle->dle.flags	= DLE_TYPE_IMAGE;
		imageDle->dle.y		= y;
		imageDle->x			= x;
		imageDle->image		= i;
		imageDle->palette	= (const uint16_t*)GET_FLASH_PTR(i->paletteOffset);
		imageDle->pixels	= (const uint8_t*)GET_FLASH_PTR(i->pixelsOffset);

		insertPendingDrawListEntry(&imageDle->dle);
		updateDrawListBounds(x, y, x + i->width, y + i->height);
	}
}

void rendererRenderDrawList() {
	if (drawListMaxX > drawListMinX) {
		tftStartBlit(drawListMinX, drawListMinY, drawListMaxX - drawListMinX, drawListMaxY - drawListMinY);
		PROFILE_ENTER(render);

		for(uint16_t y = drawListMinY; y < drawListMaxY; y++) {
			renderScanLine(y);
			PROFILE_ENTER(blit);
			tftBlit(pixelBuffer, drawListMaxX - drawListMinX);
			PROFILE_EXIT(blit);
		}

		PROFILE_EXIT(render);
		tftEndBlit();
		tftSetBacklight(1);

		PROFILE_ENTER(profileOuter);
		PROFILE_ENTER(profileInner);
		PROFILE_EXIT(profileInner);
		PROFILE_EXIT(profileOuter);

		PROFILE_END;
		PROFILE_REPORT(drawlist);
		PROFILE_REPORT(render);
		PROFILE_REPORT(scanline);
		PROFILE_REPORT(clearline);
		PROFILE_REPORT(activationCheck);
		PROFILE_REPORT(primitives);
		PROFILE_REPORT(hline);
		PROFILE_REPORT(vline);
		PROFILE_REPORT(rect);
		PROFILE_REPORT(text);
		PROFILE_REPORT(image);
		PROFILE_REPORT(blit);
		PROFILE_REPORT(profileOuter);
	}
}

void rendererGetStringBounds(const char* s, const Font* font, uint16_t* width, uint16_t* height)
{
	char c;
	*width = 0;
	*height = 0;

	while ((c = *s++)) {
		const Glyph* glyph = findGlyph(c, font);

		if (!glyph) {
			glyph = findGlyph('*', font);
		}

		if (glyph) {
			*width += glyph->width;
			*height = MAX(*height, glyph->height);
		}
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

	rendererDrawRect(8, 80, 1, 1, 0xffff);
	rendererDrawRect(9, 81, 1, 1, 0xffff);
	rendererDrawRect(8, 82, 2, 1, 0xffff);
	rendererDrawRect(9, 83, 2, 1, 0xffff);
	rendererDrawRect(8, 84, 3, 1, 0xffff);
	rendererDrawRect(9, 85, 3, 1, 0xffff);
	rendererDrawRect(8, 86, 4, 1, 0xffff);
	rendererDrawRect(9, 87, 4, 1, 0xffff);
	rendererDrawRect(8, 88, 5, 1, 0xffff);
	rendererDrawRect(9, 89, 5, 1, 0xffff);
	rendererDrawRect(8, 90, 6, 1, 0xffff);
	rendererDrawRect(9, 91, 6, 1, 0xffff);

//	rendererDrawImage(&PlayButton, 32, 126);
//	rendererDrawImage(&PlayButton, 33, 190);

	rendererRenderDrawList();
}


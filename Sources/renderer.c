/*
 * renderer.c
 *
 *  Created on: 10 May 2015
 *      Author: ntuckett
 */
#include "renderer.h"
#include <string.h>
#include "lcd.h"

#define DRAWLIST_BUFFER_SIZE	1024

#define DLE_FLAG_ACTIVE_MASK	0x01
#define DLE_FLAG_DRAWN_MASK		0x02
#define DLE_TYPE_MASK			0xf0

#define DLE_TYPE_VLINE	0x10
#define DLE_TYPE_HLINE	0x20
#define DLE_TYPE_RECT	0x30

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

#define SWAPNUM(a, b) a = a ^ b; b = a ^ b; a = a ^ b
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

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
	rendererRenderDrawList();
}


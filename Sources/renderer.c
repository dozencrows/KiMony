/*
 * renderer.c
 *
 *  Created on: 10 May 2015
 *      Author: ntuckett
 */
#include "renderer.h"
#include <string.h>
#include <stdio.h>
#include "lcd.h"
#include "systick.h"
#include "mathutil.h"
#include "fontdata.h"

#define PROFILING

#ifdef PROFILING
#define PROFILE_CATEGORY_OVERHEAD 69
#define PROFILE_CATEGORY(category) uint32_t ctr_##category; uint32_t calls_##category
#define PROFILE_BEGIN memset(&renderMetrics, 0, sizeof(renderMetrics)); sysTickStartCycleCount()
#define PROFILE_ENTER(category) uint32_t profileMark_##category = sysTickGetCycleCount()
#define PROFILE_EXIT(category) renderMetrics.ctr_##category += sysTickGetCycleCount() - profileMark_##category; renderMetrics.calls_##category++
#define PROFILE_END sysTickStopCycleCount()
#define PROFILE_REPORT(category) printf("%s: %d, %d\n", #category, renderMetrics.ctr_##category, renderMetrics.calls_##category)
#else
#define PROFILE_BEGIN
#define PROFILE_ENTER(category)
#define PROFILE_EXIT(category)
#define PROFILE_END
#endif

#define DRAWLIST_BUFFER_SIZE	4096
#define DLE_FLAG_ACTIVE_MASK	0x01
#define DLE_FLAG_DRAWN_MASK		0x02
#define DLE_TYPE_MASK			0xf0

#define DLE_TYPE_VLINE	0x10
#define DLE_TYPE_HLINE	0x20
#define DLE_TYPE_RECT	0x30
#define DLE_TYPE_TXTCH	0x40

#ifdef PROFILING
typedef struct _RenderMetrics {
	PROFILE_CATEGORY(overall);
	PROFILE_CATEGORY(scanline);
	PROFILE_CATEGORY(clearline);
	PROFILE_CATEGORY(activationCheck);
	PROFILE_CATEGORY(rendering);
	PROFILE_CATEGORY(hline);
	PROFILE_CATEGORY(vline);
	PROFILE_CATEGORY(rect);
	PROFILE_CATEGORY(text);
	PROFILE_CATEGORY(profileOuter);
	PROFILE_CATEGORY(profileInner);
	PROFILE_CATEGORY(blit);
} RenderMetrics;

static RenderMetrics renderMetrics;
#endif

typedef struct _DrawListEntry {
	struct _DrawListEntry*	next;
	uint8_t					flags;
	uint8_t					size;
	uint16_t				y;
} DrawListEntry;

typedef struct _Line {
	DrawListEntry	dle;
	uint16_t		x;
	uint16_t		length;
	uint16_t		colour;
} Line;

typedef struct _Rect {
	DrawListEntry	dle;
	uint16_t		x;
	uint16_t		width;
	uint16_t		height;
	uint16_t		colour;
} Rect;

typedef struct _TextChar {
	DrawListEntry	dle;
	uint16_t		x;
	const Glyph*	glyph;
	uint16_t		colour;
} TextChar;

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
	//PROFILE_ENTER(scanline);

	size_t undrawnCount = 0;

	//PROFILE_ENTER(clearline);
	memset(pixelBuffer, 0, (drawListMaxX - drawListMinX)*2);
	//PROFILE_EXIT(clearline);

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

	//PROFILE_ENTER(rendering);
	while(dle) {
		DrawListEntry* nextDle = dle->next;

		switch(dle->flags & DLE_TYPE_MASK) {
			case DLE_TYPE_VLINE: {
				//PROFILE_ENTER(vline);
				Line* vLine = (Line*)dle;
				if (y == dle->y + vLine->length) {
					*lastDle = nextDle;
					dle->next = dle;
				}
				else {
					pixelBuffer[vLine->x - drawListMinX] = vLine->colour;
				}
				//PROFILE_EXIT(vline);
				break;
			}
			case DLE_TYPE_HLINE: {
				//PROFILE_ENTER(hline);
				Line* hLine = (Line*)dle;
				for (uint16_t x = 0; x < hLine->length; x++) {
					pixelBuffer[hLine->x - drawListMinX + x] = hLine->colour;
				}
				*lastDle = nextDle;
				dle->next = dle;
				//PROFILE_EXIT(hline);
				break;
			}
			case DLE_TYPE_RECT: {
				//PROFILE_ENTER(rect);
				Rect* rect = (Rect*)dle;
				if (y == dle->y + rect->height) {
					*lastDle = nextDle;
					dle->next = dle;
				}
				else {
					for (uint16_t x = 0; x < rect->width; x++) {
						pixelBuffer[rect->x - drawListMinX + x] = rect->colour;
					}
				}
				//PROFILE_EXIT(rect);
				break;
			}
			case DLE_TYPE_TXTCH: {
				//PROFILE_ENTER(text);
				TextChar* textChar = (TextChar*)dle;
				if (y == dle->y + textChar->glyph->height) {
					*lastDle = nextDle;
					dle->next = dle;
				}
				else {
					const uint8_t* char_row = textChar->glyph->data + (y - dle->y) * textChar->glyph->width;
					for (uint16_t x = 0; x < textChar->glyph->width; x++) {
						if (!char_row[x]) {
							pixelBuffer[textChar->x - drawListMinX + x] = textChar->colour;
						}
					}
				}
				//PROFILE_EXIT(text);
			}
			default: {
				break;
			}
		}

		if (dle->next == nextDle) {
			lastDle = &dle->next;
		}
		else if (nextDle == NULL) {
			activeDLETail = lastDle;
		}
		dle = nextDle;
	}
	//PROFILE_EXIT(rendering);
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
}

void rendererDrawVLine(uint16_t x, uint16_t y, uint16_t length, uint16_t colour)
{
	Line* vLine = (Line*) allocDrawListEntry(sizeof(Line));

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

void rendererDrawHLine(uint16_t x, uint16_t y, uint16_t length, uint16_t colour)
{
	Line* vLine = (Line*) allocDrawListEntry(sizeof(Line));

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
	Rect* rect = (Rect*) allocDrawListEntry(sizeof(Rect));

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
	TextChar* textChar = (TextChar*) allocDrawListEntry(sizeof(TextChar));

	if (textChar) {
		textChar->dle.flags	= DLE_TYPE_TXTCH;
		textChar->dle.y		= y;
		textChar->x			= x;
		textChar->colour	= colour;
		textChar->glyph		= glyph;

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
		PROFILE_BEGIN;
		PROFILE_ENTER(overall);

		for(uint16_t y = drawListMinY; y < drawListMaxY; y++) {
			renderScanLine(y);
			//PROFILE_ENTER(blit);
			tftBlit(pixelBuffer, drawListMaxX - drawListMinX);
			//PROFILE_EXIT(blit);
		}

		PROFILE_EXIT(overall);
		tftEndBlit();

		PROFILE_ENTER(profileOuter);
		PROFILE_ENTER(profileInner);
		PROFILE_EXIT(profileInner);
		PROFILE_EXIT(profileOuter);

		PROFILE_END;
		PROFILE_REPORT(overall);
		PROFILE_REPORT(scanline);
		PROFILE_REPORT(clearline);
		PROFILE_REPORT(activationCheck);
		PROFILE_REPORT(rendering);
		PROFILE_REPORT(hline);
		PROFILE_REPORT(vline);
		PROFILE_REPORT(rect);
		PROFILE_REPORT(text);
		PROFILE_REPORT(blit);
		PROFILE_REPORT(profileOuter);
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


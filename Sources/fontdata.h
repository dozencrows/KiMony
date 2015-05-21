/*
 * fontdata.h
 *
 *  Created on: 21 May 2015
 *      Author: ntuckett
 */

#ifndef FONTDATA_H_
#define FONTDATA_H_

#include <stdint.h>

typedef struct _Glyph {
	const uint8_t *data;
	uint8_t width;
	uint8_t height;
} Glyph;

typedef struct _Character {
	long int code;
	const Glyph *image;
} Character;

typedef struct _Font {
	int length;
	const Character *chars;
} Font;


#endif /* FONTDATA_H_ */

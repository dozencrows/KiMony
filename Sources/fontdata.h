/*
 * fontdata.h
 *
 *  Created on: 21 May 2015
 *      Author: ntuckett
 */

#ifndef FONTDATA_H_
#define FONTDATA_H_

#include <stdint.h>

typedef struct {
	const uint8_t *data;
	uint8_t width;
	uint8_t height;
} Glyph;

typedef struct {
	long int code;
	const Glyph *image;
} Character;

typedef struct {
	int length;
	const Character *chars;
} Font;


#endif /* FONTDATA_H_ */

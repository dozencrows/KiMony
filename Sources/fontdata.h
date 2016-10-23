//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * fontdata.h
 *
 *  Created on: 21 May 2015
 *      Author: ntuckett
 */

#ifndef FONTDATA_H_
#define FONTDATA_H_

#include <stdint.h>

typedef struct _Glyph
{
    const uint8_t *data;
    uint8_t width;
    uint8_t height;
} Glyph;

typedef struct _Character
{
    long int code;
    const Glyph *image;
} Character;

typedef struct _Font
{
    int length;
    const Character *chars;
} Font;

extern const Font KiMony;

#endif /* FONTDATA_H_ */

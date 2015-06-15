//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * image.h
 *
 *  Created on: 23 May 2015
 *      Author: ntuckett
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include <stdint.h>

typedef struct _Image {
	uint16_t width;
	uint16_t height;
	uint32_t paletteOffset;
	uint32_t pixelsOffset;
} Image;

#endif /* IMAGE_H_ */

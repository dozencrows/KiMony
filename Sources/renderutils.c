//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * renderutils.c
 *
 *  Created on: 4 Jun 2015
 *      Author: ntuckett
 */

#include "renderer.h"
#include "fontdata.h"

void renderMessage(const char* message, uint16_t colour)
{
	rendererClearScreen();
	uint16_t width, height, x, y;

	rendererGetStringBounds(message, &KiMony, &width, &height);
	x = (SCREEN_WIDTH - width) / 2;
	y = (SCREEN_HEIGHT - height) / 2;

	rendererNewDrawList();
	rendererDrawRect(x, y, width, height, 0);
	rendererDrawString(message, x, y, &KiMony, colour);
	rendererRenderDrawList();
}



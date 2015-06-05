//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * activity.h
 *
 *  Created on: 23 May 2015
 *      Author: ntuckett
 */

#ifndef ACTIVITY_H_
#define ACTIVITY_H_
#include <stdint.h>

typedef struct _TouchButtonPage {
	int touchButtonCount;
	uint32_t touchButtonOffset;
} TouchButtonPage;

typedef struct _Activity {
	int buttonMappingCount;
	uint32_t buttonMappingOffset;
	int touchButtonPageCount;
	uint32_t touchButtonPagesOffset;
	int deviceStateCount;
	uint32_t deviceStatesOffset;
} Activity;

#endif /* ACTIVITY_H_ */

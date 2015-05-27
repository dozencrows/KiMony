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
} Activity;

#endif /* ACTIVITY_H_ */

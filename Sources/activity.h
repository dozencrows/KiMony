/*
 * activity.h
 *
 *  Created on: 23 May 2015
 *      Author: ntuckett
 */

#ifndef ACTIVITY_H_
#define ACTIVITY_H_

typedef struct _ButtonMapping ButtonMapping;
typedef struct _TouchButton TouchButton;

typedef struct _TouchButtonPage {
	int touchButtonCount;
	const TouchButton* touchButtons;
} TouchButtonPage;

typedef struct _Activity {
	int buttonMappingCount;
	const ButtonMapping* buttonMapping;
	int touchButtonPageCount;
	const TouchButtonPage* touchButtonPages;
} Activity;

#endif /* ACTIVITY_H_ */

/*
 * event.h
 *
 *  Created on: 23 May 2015
 *      Author: ntuckett
 */

#ifndef EVENT_H_
#define EVENT_H_

#define EVENT_NONE			0
#define EVENT_IRACTION		1

typedef struct _IrAction IrAction;

typedef struct _Event {
	uint32_t	type;
	union {
		const IrAction* irAction;
	};
} Event;

#endif /* EVENT_H_ */

//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

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
#define EVENT_ACTIVITY		2
#define EVENT_NEXTPAGE		3
#define EVENT_PREVPAGE		4
#define EVENT_HOME			5
#define EVENT_DOWNLOAD		6
#define EVENT_POWEROFF		7
#define EVENT_KEEPAWAKE		8

typedef struct _IrAction IrAction;
typedef struct _Activity Activity;

typedef struct _Event {
	uint32_t	type;
	union {
		struct {
			uint32_t irActionOffset;
			uint32_t deviceOffset;
		};
		uint32_t activityOffset;
	};
} Event;

#endif /* EVENT_H_ */

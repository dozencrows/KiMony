//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * ports.h
 *
 *  Created on: 7 May 2015
 *      Author: ntuckett
 */

#ifndef PORTS_H_
#define PORTS_H_

#include "MKL26Z4.h"

typedef struct _PortConfig {
	PORT_MemMapPtr 	port;
	uint32_t		pcr_mask;
	uint32_t		pcr_bits;
	int				pin_count;
	uint8_t			pins[];
} PortConfig;

typedef const PortConfig* PortConfigPtr;

extern void portInitialise(PortConfigPtr config);

#endif /* PORT_UTIL_H_ */

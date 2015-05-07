/*
 * port_util.h
 *
 *  Created on: 7 May 2015
 *      Author: ntuckett
 */

#ifndef PORT_UTIL_H_
#define PORT_UTIL_H_

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

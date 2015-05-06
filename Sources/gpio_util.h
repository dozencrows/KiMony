/*
 * gpio_util.h
 *
 *  Created on: 6 May 2015
 *      Author: ntuckett
 */

#ifndef GPIO_UTIL_H_
#define GPIO_UTIL_H_

#include <stddef.h>
#include "MKL26Z4.h"

#define GPIO_PULLNONE	0
#define GPIO_PULLUP		1
#define GPIO_PULLDOWN	-1

typedef struct _GpioPortRef {
	PORT_MemMapPtr 	port;
	uint32_t		pin;
} GpioPortRef;

typedef const GpioPortRef* GpioPortRefPtr;

extern void gpioInitPorts(GpioPortRefPtr ports, size_t count, int pullType);

#endif /* GPIO_UTIL_H_ */

/*
 * gpio_util.c
 *
 *  Created on: 6 May 2015
 *      Author: ntuckett
 */

#include "gpio_util.h"

void gpioInitPorts(const GpioPortRefPtr ports, size_t count, int pullType)
{
	uint32_t mask	= PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x06);
	uint32_t set 	= PORT_PCR_MUX(0x01);

	if (pullType > 0) {
		set  |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK;
	}
	else if (pullType < 0) {
		mask |= PORT_PCR_PS_MASK;
		set  |= PORT_PCR_PE_MASK;
	}

	mask = ~mask;
	for(size_t i = 0; i < count; i++) {
		PORT_PCR_REG(ports[i].port, ports[i].pin) = (PORT_PCR_REG(ports[i].port, ports[i].pin) & mask) | set;
	}
}

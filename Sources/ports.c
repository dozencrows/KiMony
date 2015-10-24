//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * port_util.c
 *
 *  Created on: 7 May 2015
 *      Author: ntuckett
 */
#include "ports.h"

static PORT_Type* const portBases[] = PORT_BASE_PTRS;
static FGPIO_Type* const gpioBases[] = FGPIO_BASE_PTRS;

void portInitialise(PortConfigPtr config)
{
	PORT_MemMapPtr port	= config->port;
	uint32_t mask	= config->pcr_mask;
	uint32_t set 	= config->pcr_bits;

	for(int i = 0; i < config->pin_count; i++) {
		PORT_PCR_REG(port, config->pins[i]) = (PORT_PCR_REG(port, config->pins[i]) & mask) | set;
	}
}

void portAsGPIOOutput(PortConfigPtr config)
{
	int portIndex = -1;
	for(int i = 0; i < 5; i++) {
		if (config->port == portBases[i]) {
			portIndex = i;
			break;
		}
	}

	if (portIndex > -1) {
		FGPIO_Type* gpio = gpioBases[portIndex];

		for(int i = 0; i < config->pin_count; i++) {
			uint32_t pinMask = 1 << config->pins[i];
			FGPIO_PDDR_REG(gpio) |= pinMask;
			FGPIO_PCOR_REG(gpio) = pinMask;
		}
	}
}

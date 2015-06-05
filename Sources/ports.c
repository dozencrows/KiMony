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

void portInitialise(PortConfigPtr config)
{
	PORT_MemMapPtr port	= config->port;
	uint32_t mask	= config->pcr_mask;
	uint32_t set 	= config->pcr_bits;

	for(int i = 0; i < config->pin_count; i++) {
		PORT_PCR_REG(port, config->pins[i]) = (PORT_PCR_REG(port, config->pins[i]) & mask) | set;
	}
}

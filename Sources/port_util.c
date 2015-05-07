/*
 * port_util.c
 *
 *  Created on: 7 May 2015
 *      Author: ntuckett
 */
#include "port_util.h"

void portInitialise(PortConfigPtr config)
{
	PORT_MemMapPtr port	= config->port;
	uint32_t mask	= config->pcr_mask;
	uint32_t set 	= config->pcr_bits;

	for(int i = 0; i < config->pin_count; i++) {
		PORT_PCR_REG(port, config->pins[i]) = (PORT_PCR_REG(port, config->pins[i]) & mask) | set;
	}
}

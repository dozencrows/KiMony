/*
 * capslider.c
 *
 *  Created on: 7 Nov 2015
 *      Author: ntuckett
 */

#include <stdio.h>
#include "MKL26Z4.h"
#include "ports.h"
#include "capelectrode.h"

// Pins to initialise
// PTB: 18, 19

static const PortConfig portBPins =
{
	PORTB_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(0),
	2,
	{ 18, 19 }
};

static Electrode electrodes[2];

void capSliderInit()
{
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;

	portInitialise(&portBPins);

	electrodes[0].channel = 12;
	electrodes[1].channel = 11;

	capElectrodeInit();
	capElectrodeSetElectrodes(2, electrodes);
}

uint8_t capSliderGetPercentage()
{
	uint32_t c0 = capElectrodeGetValue(0);
	uint32_t c1 = capElectrodeGetValue(1);

	if (c0 > 0 || c1 > 0) {
		uint32_t percentage0 = (c0 * 100) / (c0 + c1);
		uint32_t percentage1 = (c1 * 100) / (c0 + c1);
		return ((100 - percentage0) + percentage1) / 2;
	} else {
		return 0;
	}
}

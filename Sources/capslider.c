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

	electrodes[0].channel = 11;
	electrodes[1].channel = 12;

	capElectrodeInit();
	capElectrodeSetElectrodes(2, electrodes);
}

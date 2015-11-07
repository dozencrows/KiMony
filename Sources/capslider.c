/*
 * capslider.c
 *
 *  Created on: 7 Nov 2015
 *      Author: ntuckett
 */

#include <stdio.h>
#include "MKL26Z4.h"
#include "ports.h"

#define TSI_SCAN_COUNT		32

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

static volatile uint8_t capSliderIntFlag = 0;
static volatile uint16_t capSliderCounter = 0;

void capSliderInit()
{
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK|SIM_SCGC5_TSI_MASK;

	portInitialise(&portBPins);

	TSI0_GENCS = // Fields to clear
				 TSI_GENCS_OUTRGF_MASK | TSI_GENCS_EOSF_MASK |
				 // Fields to set: int on end scan, 16uA ref charge, 8uA ext charge, 0.73V DVolt, 31 scans, enable interrupt & module
				 TSI_GENCS_ESOR_MASK | TSI_GENCS_REFCHRG(5) | TSI_GENCS_DVOLT(1) | TSI_GENCS_EXTCHRG(4) |
				 TSI_GENCS_PS(1) | TSI_GENCS_NSCN(TSI_SCAN_COUNT - 1) | TSI_GENCS_TSIEN_MASK | TSI_GENCS_TSIIEN_MASK;
	TSI0_TSHD  = TSI_TSHD_THRESL(100) | TSI_TSHD_THRESH(200);

	NVIC_EnableIRQ(TSI0_IRQn);
}

void TSI0_IRQHandler()
{
	capSliderCounter = (TSI0_DATA & TSI_DATA_TSICNT_MASK);
	TSI0_GENCS |= TSI_GENCS_EOSF_MASK;
	capSliderIntFlag++;
}

void capSliderStartRead(int channel)
{
	capSliderIntFlag = 0;
	TSI0_DATA = TSI_DATA_TSICH(channel) | TSI_DATA_SWTS_MASK;
}

int capSliderReadDone()
{
	return capSliderIntFlag != 0;
}

int capSliderValue()
{
	return capSliderCounter / TSI_SCAN_COUNT;
}


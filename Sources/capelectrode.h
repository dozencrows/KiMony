/*
 * capelectrode.h
 *
 *  Created on: 8 Nov 2015
 *      Author: ntuckett
 */

#ifndef CAPELECTRODE_H_
#define CAPELECTRODE_H_

#include <stdint.h>

// Flag values
#define ELECTRODE_FLAGS_ACTIVE	0x0001	// Electrode has been initialised and is active

#define ELECTRODE_BUFFER_SIZE	4

typedef struct _Electrode {
	uint8_t 	channel;
	uint8_t		flags;
	uint16_t	baseline;

	uint16_t	buffer[ELECTRODE_BUFFER_SIZE];
	uint32_t	bufferWriteIndex;
} Electrode;

extern void capElectrodeInit();
extern void capElectrodeSleep();
extern void capElectrodeWake();
extern void capElectrodeSetElectrodes(int count, Electrode* electrodes);
extern uint16_t capElectrodeGetValue(int electrode);

#endif /* CAPELECTRODE_H_ */

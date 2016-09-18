/*
 * capelectrode.c
 *
 *  Created on: 8 Nov 2015
 *      Author: ntuckett
 */
#include "capelectrode.h"
#include "MKL26Z4.h"
#include <stddef.h>
#include "timer.h"

#define TSI_SCAN_COUNT		16
#define TPM_TIMER			1

static int electrodeCount = 0;
static Electrode* electrodeList = NULL;
static Electrode* activeElectrode = NULL;
static uint8_t isSleeping = 0;
static uint8_t outOfRangeInterruptCount = 0;

void TSI0_IRQHandler()
{
	if (TSI0_GENCS & TSI_GENCS_OUTRGF_MASK && isSleeping) {
		TSI0_GENCS |= TSI_GENCS_OUTRGF_MASK | TSI_GENCS_EOSF_MASK;
		outOfRangeInterruptCount++;
	} else {
#if defined(CAPELECTRODE_TIMESTAMPS)
		activeElectrode->timestamp[activeElectrode->bufferWriteIndex] = tpmGetTimeHighPrecision(TPM_TIMER);
#endif
		activeElectrode->buffer[activeElectrode->bufferWriteIndex++] = (TSI0_DATA & TSI_DATA_TSICNT_MASK) / TSI_SCAN_COUNT;
		TSI0_GENCS |= TSI_GENCS_EOSF_MASK;
		activeElectrode->bufferWriteIndex %= ELECTRODE_BUFFER_SIZE;

		if (!(activeElectrode->flags & ELECTRODE_FLAGS_ACTIVE) && !activeElectrode->bufferWriteIndex) {
			activeElectrode->baseline = (activeElectrode->buffer[0] + activeElectrode->buffer[1] + activeElectrode->buffer[2] + activeElectrode->buffer[3]) / 4;
			activeElectrode->flags |= ELECTRODE_FLAGS_ACTIVE;
		}

		activeElectrode++;
		if (electrodeCount == activeElectrode - electrodeList) {
			activeElectrode = electrodeList;
		}
		TSI0_DATA = TSI_DATA_TSICH(activeElectrode->channel) | TSI_DATA_SWTS_MASK;
	}
}

void capElectrodeInit()
{
#if defined(CAPELECTRODE_TIMESTAMPS)
	tpmStartTimer(TPM_TIMER, TPM_CLOCKS_PER_MILLISECOND, 0);
#endif

	SIM_SCGC5 |= SIM_SCGC5_TSI_MASK;

	TSI0_GENCS = // Fields to clear
				 TSI_GENCS_OUTRGF_MASK | TSI_GENCS_EOSF_MASK |
				 // Fields to set: int on end scan, 16uA ref charge, 8uA ext charge, 0.43V DVolt, X scans, enable interrupt & module, enable in low power
				 TSI_GENCS_ESOR_MASK | TSI_GENCS_REFCHRG(5) | TSI_GENCS_DVOLT(2) | TSI_GENCS_EXTCHRG(4) | TSI_GENCS_STPE_MASK |
				 TSI_GENCS_PS(1) | TSI_GENCS_NSCN(TSI_SCAN_COUNT - 1) | TSI_GENCS_TSIEN_MASK | TSI_GENCS_TSIIEN_MASK;
}

void capElectrodeSleep()
{
	TSI0_GENCS &= ~TSI_GENCS_STPE_MASK;
}

void capElectrodeWakeableSleep(int electrodeIdx)
{
	Electrode* wakeElectrode = electrodeList + electrodeIdx;

	TSI0_GENCS &= ~TSI_GENCS_TSIEN_MASK;

	isSleeping  = 1;
	TSI0_TSHD  	= TSI_TSHD_THRESL(wakeElectrode->baseline / 2) | TSI_TSHD_THRESH(wakeElectrode->baseline + 5);
	TSI0_DATA = TSI_DATA_TSICH(wakeElectrode->channel);
	TSI0_GENCS 	= // Fields to clear
				 TSI_GENCS_OUTRGF_MASK | TSI_GENCS_EOSF_MASK |
				 // Fields to set: 16uA ref charge, 8uA ext charge, 0.43V DVolt, 1 scans, enable interrupt & module, enable in low power, hardware trigger
				 TSI_GENCS_REFCHRG(5) | TSI_GENCS_DVOLT(2) | TSI_GENCS_EXTCHRG(4) | TSI_GENCS_STPE_MASK |
				 TSI_GENCS_PS(1) | TSI_GENCS_NSCN(0) | TSI_GENCS_TSIEN_MASK | TSI_GENCS_TSIIEN_MASK | TSI_GENCS_STM_MASK;
}

int capElectrodeCheckWakeInterrupt()
{
	return outOfRangeInterruptCount > 0;
}

void capElectrodeClearWakeInterrupt()
{
	outOfRangeInterruptCount = 0;
}

void capElectrodeWake()
{
	capElectrodeInit();
	isSleeping = 0;
	if (activeElectrode) {
		TSI0_DATA = TSI_DATA_TSICH(activeElectrode->channel) | TSI_DATA_SWTS_MASK;
	}
}

void capElectrodeSetElectrodes(int count, Electrode* electrodes)
{
	NVIC_DisableIRQ(TSI0_IRQn);

	if (count > 0 && electrodes != NULL) {
		electrodeList = electrodes;
		electrodeCount = count;

		for (int i = 0; i < count; i++) {
			electrodes[i].flags = 0;
			electrodes[i].baseline = 0;
			electrodes[i].bufferWriteIndex = 0;
		}

		activeElectrode = electrodeList;
		NVIC_EnableIRQ(TSI0_IRQn);
		TSI0_DATA = TSI_DATA_TSICH(activeElectrode->channel) | TSI_DATA_SWTS_MASK;
	}
	else {
		electrodeList = NULL;
		electrodeCount = 0;
	}
}

uint16_t capElectrodeGetValue(int electrodeIdx)
{
	if (electrodeList != NULL && electrodeIdx < electrodeCount) {
		Electrode* electrode = electrodeList + electrodeIdx;

		if (electrode->flags & ELECTRODE_FLAGS_ACTIVE) {
			uint16_t average = (electrode->buffer[0] + electrode->buffer[1] + electrode->buffer[2] + electrode->buffer[3]) / 4;
			if (average > electrode->baseline) {
				return average - electrode->baseline;
			}
		}
	}

	return 0;
}


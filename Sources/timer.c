/*
 * timer.c
 *
 *  Created on: 11 Jun 2015
 *      Author: ntuckett
 */
#include "timer.h"
#include "MKL26Z4.h"

volatile uint32_t tpmCounter[3] = { 0, 0, 0 };
TPM_Type* const tpmPtrs[] = TPM_BASE_PTRS;
const uint32_t tpmGateFlags[] = { SIM_SCGC6_TPM0_MASK, SIM_SCGC6_TPM1_MASK, SIM_SCGC6_TPM2_MASK };

void TPM0_IRQHandler()
{
	tpmCounter[0]++;
	tpmPtrs[0]->SC |= TPM_SC_TOF_MASK;
}

void TPM1_IRQHandler()
{
	tpmCounter[1]++;
	tpmPtrs[1]->SC |= TPM_SC_TOF_MASK;
}

void TPM2_IRQHandler()
{
	tpmCounter[2]++;
	tpmPtrs[2]->SC |= TPM_SC_TOF_MASK;
}

void tpmInit()
{
	SIM_SOPT2 = (SIM_SOPT2 & ~SIM_SOPT2_TPMSRC_MASK) | SIM_SOPT2_TPMSRC(1);	// PLL / 2 clock
}

void tpmStartTimer(int timerIndex, uint32_t periodClocks, uint32_t prescaleShift)
{
	SIM_SCGC6 |= tpmGateFlags[timerIndex];
	tpmPtrs[timerIndex]->MOD = periodClocks >> (prescaleShift & TPM_SC_PS_MASK);
	tpmPtrs[timerIndex]->CNT = 0;
	tpmPtrs[timerIndex]->SC  = TPM_SC_TOIE_MASK|TPM_SC_TOF_MASK|TPM_SC_PS(prescaleShift);
	tpmCounter[timerIndex] = 0;
	NVIC_EnableIRQ(TPM0_IRQn + timerIndex);
	tpmPtrs[timerIndex]->SC |= TPM_SC_CMOD(1);
}

uint32_t tpmGetTime(int timerIndex)
{
	return tpmCounter[timerIndex];
}

void tpmStopTimer(int timerIndex)
{
	tpmPtrs[timerIndex]->SC = 0;
	NVIC_DisableIRQ(TPM0_IRQn + timerIndex);
	SIM_SCGC6 &= ~tpmGateFlags[timerIndex];
}

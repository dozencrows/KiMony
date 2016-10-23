/*
 * timer.c
 *
 *  Created on: 11 Jun 2015
 *      Author: ntuckett
 */
#include "timer.h"
#include "MKL26Z4.h"
#include "interrupts.h"

volatile uint32_t tpmCounter[3] = { 0, 0, 0 };
TPM_Type* const tpmPtrs[] = TPM_BASE_PTRS;
const uint32_t tpmGateFlags[] = { SIM_SCGC6_TPM0_MASK, SIM_SCGC6_TPM1_MASK, SIM_SCGC6_TPM2_MASK };

void timerTPMIRQHandler(uint32_t tpm)
{
    tpmCounter[tpm]++;
}

void tpmInit()
{
#if defined(TPM_CLOCK_IRCLK)
    MCG_SC &= ~MCG_SC_FCRDIV_MASK;										    // zero fast IR clock divider
    MCG_C1 |= MCG_C1_IRCLKEN_MASK | MCG_C1_IREFSTEN_MASK;					// enable MCGIRCLK, also in stop mode
    MCG_C2 |= MCG_C2_IRCS_MASK;											    // select fast MCGIRCLK
    SIM_SOPT2 = (SIM_SOPT2 & ~SIM_SOPT2_TPMSRC_MASK) | SIM_SOPT2_TPMSRC(3);	// select MCGIRCLK for TPM source
#else
    SIM_SOPT2 = (SIM_SOPT2 & ~SIM_SOPT2_TPMSRC_MASK) | SIM_SOPT2_TPMSRC(1);	// PLL / 2 clock
#endif

    interruptRegisterTPMIRQHandler(timerTPMIRQHandler, 0);
    interruptRegisterTPMIRQHandler(timerTPMIRQHandler, 1);
    interruptRegisterTPMIRQHandler(timerTPMIRQHandler, 2);
}

void tpmEnableTimer(int timerIndex)
{
    SIM_SCGC6 |= tpmGateFlags[timerIndex];
    NVIC_EnableIRQ(TPM0_IRQn + timerIndex);
}

void tpmStartTimer(int timerIndex, uint32_t periodClocks, uint32_t prescaleShift)
{
    tpmPtrs[timerIndex]->MOD = (periodClocks >> (prescaleShift & TPM_SC_PS_MASK)) - 1;
    tpmPtrs[timerIndex]->CNT = 0;
    tpmPtrs[timerIndex]->SC = TPM_SC_TOIE_MASK | TPM_SC_TOF_MASK | TPM_SC_PS(prescaleShift);
    tpmPtrs[timerIndex]->CONF = 0;
    tpmCounter[timerIndex] = 0;
    tpmPtrs[timerIndex]->SC |= TPM_SC_CMOD(1);
}

void tpmOneShotTimer(int timerIndex, uint32_t periodClocks, uint32_t prescaleShift)
{
    SIM_SCGC6 |= tpmGateFlags[timerIndex];
    tpmPtrs[timerIndex]->MOD = (periodClocks >> (prescaleShift & TPM_SC_PS_MASK)) - 1;
    tpmPtrs[timerIndex]->CNT = 0;
    tpmPtrs[timerIndex]->SC = TPM_SC_TOIE_MASK | TPM_SC_TOF_MASK | TPM_SC_PS(prescaleShift);
    tpmPtrs[timerIndex]->CONF = TPM_CONF_CSOO_MASK;
    tpmCounter[timerIndex] = 0;
    NVIC_EnableIRQ(TPM0_IRQn + timerIndex);
    tpmPtrs[timerIndex]->SC |= TPM_SC_CMOD(1);
}

uint32_t tpmGetTime(int timerIndex)
{
    return tpmCounter[timerIndex];
}

uint32_t tpmGetTimeHighPrecision(int timerIndex)
{
    uint32_t timerValue = tpmPtrs[timerIndex]->CNT;
    uint32_t countValue = tpmCounter[timerIndex];
    uint32_t timerValue2 = tpmPtrs[timerIndex]->CNT;

    if (timerValue2 < timerValue) {
        countValue = tpmCounter[timerIndex];
        timerValue = timerValue2;
    }

    return countValue * tpmPtrs[timerIndex]->MOD + timerValue;
}

void tpmStopTimer(int timerIndex)
{
    tpmPtrs[timerIndex]->SC = 0;
}

void tpmDisableTimer(int timerIndex)
{
    NVIC_DisableIRQ(TPM0_IRQn + timerIndex);
    SIM_SCGC6 &= ~tpmGateFlags[timerIndex];
}

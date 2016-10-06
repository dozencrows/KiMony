//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * interrupts.c
 *
 *  Created on: 15 May 2015
 *      Author: ntuckett
 */
#include "MKL26Z4.h"
#include "interrupts.h"

#define MAX_IRQ_HANDLERS	4
#define MAX_TPM				3

extern TPM_Type* const tpmPtrs[];

static int lptmrIRQHandlerCount = 0;
static PITIRQHandler lptmrIRQHandlers[MAX_IRQ_HANDLERS];
static int pitIRQHandlerCount = 0;
static PITIRQHandler pitIRQHandlers[MAX_IRQ_HANDLERS];
static int portAIRQHandlerCount = 0;
static PortAIRQHandler portAIRQHandlers[MAX_IRQ_HANDLERS];
static int portCDIRQHandlerCount = 0;
static PortCDIRQHandler portCDIRQHandlers[MAX_IRQ_HANDLERS];
static int tpmIRQHandlerCount[MAX_TPM] = { 0, 0, 0 };
static TPMIRQHandler tpmIRQHandlers[MAX_TPM][MAX_IRQ_HANDLERS];

void LPTimer_IRQHandler()
{
	for(int i = 0; i < lptmrIRQHandlerCount; i++) {
		lptmrIRQHandlers[i]();
	}

	LPTMR0_CSR |= LPTMR_CSR_TCF_MASK;
}

void PIT_IRQHandler()
{
	for(int i = 0; i < pitIRQHandlerCount; i++) {
		pitIRQHandlers[i]();
	}

	PIT_TFLG0 = PIT_TFLG_TIF_MASK;
	PIT_TFLG1 = PIT_TFLG_TIF_MASK;
}

void TPM_IRQHandler(uint32_t tpm)
{
	for(int i = 0; i < tpmIRQHandlerCount[tpm]; i++) {
		tpmIRQHandlers[tpm][i](tpm);
	}

	tpmPtrs[tpm]->SC |= TPM_SC_TOF_MASK;
}

void TPM0_IRQHandler()
{
	TPM_IRQHandler(0);
}

void TPM1_IRQHandler()
{
	TPM_IRQHandler(1);
}

void TPM2_IRQHandler()
{
	TPM_IRQHandler(2);
}


void PORTA_IRQHandler()
{
	uint32_t portAISFR = PORTA_ISFR;

	for(int i = 0; i < portAIRQHandlerCount; i++) {
		portAIRQHandlers[i](portAISFR);
	}

	PORTA_ISFR = portAISFR;
}

void PORTC_PORTD_IRQHandler()
{
	uint32_t portCISFR = PORTC_ISFR;
	uint32_t portDISFR = PORTD_ISFR;

	for(int i = 0; i < portCDIRQHandlerCount; i++) {
		portCDIRQHandlers[i](portCISFR, portDISFR);
	}

	PORTC_ISFR = portCISFR;
	PORTD_ISFR = portDISFR;
}

static void addIrqHandler(void* irqHandler, int* irqHandlerCount, void** irqHandlerArray)
{
	if (*irqHandlerCount < MAX_IRQ_HANDLERS) {
		irqHandlerArray[*irqHandlerCount] = irqHandler;
		*irqHandlerCount = *irqHandlerCount + 1;
	}
}

void interruptRegisterLPTMRIRQHandler(LPTMRIRQHandler irqHandler)
{
	addIrqHandler(irqHandler, &lptmrIRQHandlerCount, (void**)lptmrIRQHandlers);
}

void interruptRegisterPITIRQHandler(PITIRQHandler irqHandler)
{
	addIrqHandler(irqHandler, &pitIRQHandlerCount, (void**)pitIRQHandlers);
}

void interruptRegisterTPMIRQHandler(TPMIRQHandler irqHandler, uint32_t tpm)
{
	addIrqHandler(irqHandler, tpmIRQHandlerCount + tpm, (void**)(tpmIRQHandlers + tpm));
}

void interruptRegisterPortAIRQHandler(PortAIRQHandler irqHandler)
{
	addIrqHandler(irqHandler, &portAIRQHandlerCount, (void**)portAIRQHandlers);
}

void interruptRegisterPortCDIRQHandler(PortCDIRQHandler irqHandler)
{
	addIrqHandler(irqHandler, &portCDIRQHandlerCount, (void**)portCDIRQHandlers);
}

/*
 * interrupts.c
 *
 *  Created on: 15 May 2015
 *      Author: ntuckett
 */
#include "MKL26Z4.h"
#include "interrupts.h"

#define MAX_IRQ_HANDLERS	4

static int pitIRQHandlerCount = 0;
static PITIRQHandler pitIRQHandlers[MAX_IRQ_HANDLERS];
static int portAIRQHandlerCount = 0;
static PortAIRQHandler portAIRQHandlers[MAX_IRQ_HANDLERS];
static int portCDIRQHandlerCount = 0;
static PortCDIRQHandler portCDIRQHandlers[MAX_IRQ_HANDLERS];

void PIT_IRQHandler()
{
	for(int i = 0; i < pitIRQHandlerCount; i++) {
		pitIRQHandlers[i]();
	}

	PIT_TFLG0 = PIT_TFLG_TIF_MASK;
	PIT_TFLG1 = PIT_TFLG_TIF_MASK;
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

void interruptRegisterPITIRQHandler(PITIRQHandler irqHandler)
{
	addIrqHandler(irqHandler, &pitIRQHandlerCount, (void**)pitIRQHandlers);
}

void interruptRegisterPortAIRQHandler(PortAIRQHandler irqHandler)
{
	addIrqHandler(irqHandler, &portAIRQHandlerCount, (void**)portAIRQHandlers);
}

void interruptRegisterPortCDIRQHandler(PortCDIRQHandler irqHandler)
{
	addIrqHandler(irqHandler, &portCDIRQHandlerCount, (void**)portCDIRQHandlers);
}
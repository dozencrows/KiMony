/*
 * interrupts.h
 *
 *  Created on: 15 May 2015
 *      Author: ntuckett
 */

#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include <stdint.h>

typedef void (*PITIRQHandler)();
typedef void (*PortAIRQHandler)(uint32_t portAISFR);
typedef void (*PortCDIRQHandler)(uint32_t portCISFR, uint32_t portDISFR);

void interruptRegisterPITIRQHandler(PITIRQHandler irqHandler);
void interruptRegisterPortAIRQHandler(PortAIRQHandler irqHandler);
void interruptRegisterPortCDIRQHandler(PortCDIRQHandler irqHandler);


#endif /* INTERRUPTS_H_ */
//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * interrupts.h
 *
 *  Created on: 15 May 2015
 *      Author: ntuckett
 */

#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include <stdint.h>

typedef void (*LPTMRIRQHandler)();
typedef void (*PITIRQHandler)();
typedef void (*TPMIRQHandler)(uint32_t tpm);
typedef void (*PortAIRQHandler)(uint32_t portAISFR);
typedef void (*PortCDIRQHandler)(uint32_t portCISFR, uint32_t portDISFR);

void interruptRegisterLPTMRIRQHandler(LPTMRIRQHandler irqHandler);
void interruptRegisterPITIRQHandler(PITIRQHandler irqHandler);
void interruptRegisterTPMIRQHandler(TPMIRQHandler irqHandler, uint32_t tpm);
void interruptRegisterPortAIRQHandler(PortAIRQHandler irqHandler);
void interruptRegisterPortCDIRQHandler(PortCDIRQHandler irqHandler);


#endif /* INTERRUPTS_H_ */

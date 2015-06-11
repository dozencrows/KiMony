//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * device.h
 *
 *  Created on: 4 Jun 2015
 *      Author: ntuckett
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include <stdint.h>

typedef struct _IrAction IrAction;

#define OPTION_CYCLED				0x0001
#define OPTION_DEFAULT_TO_ZERO		0x0002
#define OPTION_ACTION_ON_DEFAULT	0x0004
#define OPTION_ALWAYS_SET			0x0008
#define OPTION_ABSOLUTE_FROM_ZERO	0x0010

typedef struct _Option {
	uint16_t	flags;
	uint8_t		maxValue;
	uint8_t		actionCount;
	uint32_t	preActionOffset;
	uint32_t	actionsOffset;
	uint32_t	postDelaysOffset;
} Option;

typedef struct _Device {
	int			optionCount;
	uint32_t	optionsOffset;
} Device;

typedef struct _DeviceState {
	uint32_t	deviceOffset;
	uint32_t	optionValuesOffset;
} DeviceState;

extern void deviceInit();
extern void deviceSetActive(const Device* devices, int deviceCount);
extern void deviceSetStates(const DeviceState* states, int stateCount);
extern void deviceDoIrAction(const Device* device, const IrAction* irAction);
extern int deviceAreAllOnDefault();

#endif /* DEVICE_H_ */

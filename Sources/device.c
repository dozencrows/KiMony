//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * device.c
 *
 *  Created on: 4 Jun 2015
 *      Author: ntuckett
 */
#include "device.h"
#include <stddef.h>
#include <string.h>

#include "ir.h"
#include "flash.h"

#define MAX_OPTIONS	64
#define MAX_DEVICES 32

typedef struct _DeviceDynamicState {
	uint8_t optionValuesOffset;
	uint8_t toggleFlag;
} DeviceDynamicState;

static const Device* activeDevices = NULL;
static int activeDeviceCount = 0;

static DeviceDynamicState deviceDynamicState[MAX_DEVICES];
static uint8_t optionValuesStore[MAX_OPTIONS];

static unsigned int getDeviceIndex(const Device* device)
{
	ptrdiff_t index = device - activeDevices;

	if (index < 0) {
		index = activeDeviceCount;
	}

	return index;
}

static void actionOptionToValue(const Device* device, const Option* option, uint8_t currentValue, uint8_t newValue)
{
	const uint32_t* actionRefs = (const uint32_t*) GET_FLASH_PTR(option->actionsOffset);

	unsigned int deviceIndex = getDeviceIndex(device);
	int toggleFlag = deviceDynamicState[deviceIndex].toggleFlag;

	if (option->preActionOffset) {
		irDoAction((const IrAction*)GET_FLASH_PTR(option->preActionOffset), &toggleFlag);
	}

	if (option->flags & OPTION_CYCLED) {
		if (option->actionCount == 1) {
			while (currentValue != newValue) {
				irDoAction((const IrAction*)GET_FLASH_PTR(actionRefs[0]), &toggleFlag);
				currentValue++;
				if (currentValue > option->maxValue) {
					currentValue = 0;
				}
			}
		}
		else {
			if (option->flags & OPTION_ABSOLUTE_FROM_ZERO && currentValue != 0) {
				irDoAction((const IrAction*)GET_FLASH_PTR(actionRefs[0]), &toggleFlag);
				currentValue = 0;
			}

			while (currentValue < newValue) {
				irDoAction((const IrAction*)GET_FLASH_PTR(actionRefs[1]), &toggleFlag);
				currentValue++;
			}

			while (currentValue > newValue) {
				irDoAction((const IrAction*)GET_FLASH_PTR(actionRefs[0]), &toggleFlag);
				currentValue--;
			}
		}
	}
	else {
		irDoAction((const IrAction*)GET_FLASH_PTR(actionRefs[newValue]), &toggleFlag);
	}

	deviceDynamicState[deviceIndex].toggleFlag = toggleFlag ? 1 : 0;
}

static void setDeviceToState(const DeviceState* state, int deviceIndex)
{
	const Device* device = activeDevices + deviceIndex;
	const Option* options = (const Option*) GET_FLASH_PTR(device->optionsOffset);
	const uint8_t* stateOptionValues = GET_FLASH_PTR(state->optionValuesOffset);
	uint8_t* optionValues = optionValuesStore + deviceDynamicState[deviceIndex].optionValuesOffset;

	for (int i = 0; i < device->optionCount; i++) {
		if (stateOptionValues[i] != optionValues[i] || (options[i].flags & OPTION_ALWAYS_SET)) {
			actionOptionToValue(device, options + i, optionValues[i], stateOptionValues[i]);
			optionValues[i] = stateOptionValues[i];
		}
	}
}

static void setDeviceToDefault(int deviceIndex)
{
	const Device* device = activeDevices + deviceIndex;
	const Option* options = (const Option*) GET_FLASH_PTR(device->optionsOffset);
	uint8_t* optionValues = optionValuesStore + deviceDynamicState[deviceIndex].optionValuesOffset;

	for (int i = 0; i < device->optionCount; i++) {
		if (options[i].flags & OPTION_DEFAULT_TO_ZERO && optionValues[i] != 0) {
			if (options[i].flags & OPTION_ACTION_ON_DEFAULT) {
				actionOptionToValue(device, options + i, optionValues[i], 0);
			}

			optionValues[i] = 0;
		}
	}
}

void deviceInit()
{
	memset(optionValuesStore, 0, sizeof(optionValuesStore));
}

void deviceSetActive(const Device* devices, int deviceCount)
{
	activeDevices = devices;
	activeDeviceCount = deviceCount;

	int i;
	size_t nextOption = 0;

	for (i = 0; i < deviceCount && nextOption < MAX_OPTIONS - devices[i].optionCount; i++) {
		deviceDynamicState[i].optionValuesOffset = nextOption;
		deviceDynamicState[i].toggleFlag = 0;
		nextOption += devices[i].optionCount;
	}

	for (; i < deviceCount; i++) {
		deviceDynamicState[i].optionValuesOffset = MAX_OPTIONS;
	}
}

void deviceSetStates(const DeviceState* states, int stateCount)
{
	for (int i = 0; i < activeDeviceCount; i++) {
		const DeviceState* stateForDevice = NULL;

		for (int j = 0; j < stateCount; j++) {
			const Device* stateDevice = (const Device*) GET_FLASH_PTR(states[j].deviceOffset);
			if (stateDevice == activeDevices + i) {
				stateForDevice = states + j;
				break;
			}
		}

		if (stateForDevice) {
			setDeviceToState(stateForDevice, i);
		}
		else {
			setDeviceToDefault(i);
		}
	}
}

void deviceDoIrAction(const Device* device, const IrAction* action)
{
	unsigned int deviceIndex = getDeviceIndex(device);
	int toggleFlag = deviceDynamicState[deviceIndex].toggleFlag;
	irDoAction(action, &toggleFlag);
	deviceDynamicState[deviceIndex].toggleFlag = toggleFlag ? 1 : 0;
}

int deviceAreAllOnDefault()
{
	for (int i = 0; i < activeDeviceCount; i++) {
		const Option* options = (const Option*) GET_FLASH_PTR(activeDevices[i].optionsOffset);

		for (int j = 0; j < activeDevices[i].optionCount; j++) {
			if (options[j].flags & OPTION_DEFAULT_TO_ZERO) {
				if (optionValuesStore[deviceDynamicState[i].optionValuesOffset + j]) {
					return 0;
				}
			}
		}
	}

	return 1;
}

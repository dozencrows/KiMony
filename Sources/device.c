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
} DeviceDynamicState;

static const Device* activeDevices = NULL;
static int activeDeviceCount = 0;

static DeviceDynamicState deviceDynamicState[MAX_DEVICES];
static uint8_t optionValuesStore[MAX_OPTIONS];

static void actionOptionToValue(const Option* option, uint8_t currentValue, uint8_t newValue)
{
	const uint32_t* actionRefs = (const uint32_t*) GET_FLASH_PTR(option->actionsOffset);

	if (option->preActionOffset) {
		irDoAction((const IrAction*)GET_FLASH_PTR(option->preActionOffset));
	}

	if (option->flags & OPTION_CYCLED) {
		if (option->actionCount == 1) {
			while (currentValue != newValue) {
				irDoAction((const IrAction*)GET_FLASH_PTR(actionRefs[0]));
				currentValue++;
				if (currentValue > option->maxValue) {
					currentValue = 0;
				}
			}
		}
		else {
			while (currentValue < newValue) {
				irDoAction((const IrAction*)GET_FLASH_PTR(actionRefs[1]));
				currentValue++;
			}

			while (currentValue > newValue) {
				irDoAction((const IrAction*)GET_FLASH_PTR(actionRefs[0]));
				currentValue--;
			}
		}
	}
	else {
		irDoAction((const IrAction*)GET_FLASH_PTR(actionRefs[newValue]));
	}
}

static void setDeviceToState(const DeviceState* state, int deviceIndex)
{
	const Device* device = activeDevices + deviceIndex;
	const Option* options = (const Option*) GET_FLASH_PTR(device->optionsOffset);
	const uint8_t* stateOptionValues = GET_FLASH_PTR(state->optionValuesOffset);
	uint8_t* optionValues = optionValuesStore + deviceDynamicState[deviceIndex].optionValuesOffset;

	for (int i = 0; i < device->optionCount; i++) {
		if (stateOptionValues[i] != optionValues[i]) {
			actionOptionToValue(options + i, optionValues[i], stateOptionValues[i]);
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
		if (options[i].flags & OPTION_DEFAULT_TO_ZERO) {
			if (options[i].flags & OPTION_ACTION_ON_DEFAULT) {
				actionOptionToValue(options + i, optionValues[i], 0);
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

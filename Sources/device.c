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
#include <alloca.h>

#include "ir.h"
#include "flash.h"
#include "timer.h"

#define MAX_OPTIONS	64
#define MAX_DEVICES 32

typedef struct _DeviceDynamicState
{
    uint8_t optionValuesOffset;
    uint8_t toggleFlag;
} DeviceDynamicState;

typedef struct _DeviceSwitchingState
{
    uint32_t finished :1;
    uint32_t delay :30;
    uint32_t currentOption;
    const DeviceState* newState;
} DeviceSwitchingState;

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

static int actionOptionToValue(const Device* device, const Option* option, uint8_t currentValue, uint8_t newValue)
{
    int actionTaken = -1;
    const uint32_t* actionRefs = (const uint32_t*) GET_FLASH_PTR(option->actionsOffset);

    unsigned int deviceIndex = getDeviceIndex(device);

    if (option->preActionOffset) {
        irDoAction((const IrAction*) GET_FLASH_PTR(option->preActionOffset), &deviceDynamicState[deviceIndex].toggleFlag);
    }

    if (option->flags & OPTION_CYCLED) {
        if (option->actionCount == 1) {
            while (currentValue != newValue) {
                irDoAction((const IrAction*) GET_FLASH_PTR(actionRefs[0]), &deviceDynamicState[deviceIndex].toggleFlag);
                currentValue++;
                if (currentValue > option->maxValue) {
                    currentValue = 0;
                }
            }

            actionTaken = 0;
        } else {
            if (option->flags & OPTION_ABSOLUTE_FROM_ZERO && currentValue != 0) {
                irDoAction((const IrAction*) GET_FLASH_PTR(actionRefs[0]), &deviceDynamicState[deviceIndex].toggleFlag);
                currentValue = 0;
            }

            while (currentValue < newValue) {
                irDoAction((const IrAction*) GET_FLASH_PTR(actionRefs[1]), &deviceDynamicState[deviceIndex].toggleFlag);
                currentValue++;
                actionTaken = 1;
            }

            while (currentValue > newValue) {
                irDoAction((const IrAction*) GET_FLASH_PTR(actionRefs[0]), &deviceDynamicState[deviceIndex].toggleFlag);
                currentValue--;
                actionTaken = 0;
            }
        }
    } else {
        irDoAction((const IrAction*) GET_FLASH_PTR(actionRefs[newValue]), &deviceDynamicState[deviceIndex].toggleFlag);

        actionTaken = newValue;
    }

    return actionTaken;
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
        } else {
            setDeviceToDefault(i);
        }
    }
}

void deviceDoIrAction(const Device* device, const IrAction* action)
{
    unsigned int deviceIndex = getDeviceIndex(device);
    irQueueAction(action, &deviceDynamicState[deviceIndex].toggleFlag);
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

static const DeviceState* getStateForDevice(const DeviceState* states, int stateCount, const Device* device)
{
    for (int j = 0; j < stateCount; j++) {
        const Device* stateDevice = (const Device*) GET_FLASH_PTR(states[j].deviceOffset);
        if (stateDevice == device) {
            return states + j;
        }
    }

    return NULL;
}

static int setDeviceOptionToState(int deviceIndex, int optionIndex, const DeviceState* state)
{
    const Device* device = activeDevices + deviceIndex;
    const uint8_t* stateOptionValues = GET_FLASH_PTR(state->optionValuesOffset);
    const Option* option = ((const Option*) GET_FLASH_PTR(device->optionsOffset)) + optionIndex;
    uint8_t* optionValues = optionValuesStore + deviceDynamicState[deviceIndex].optionValuesOffset;
    int actionTaken = -1;

    if (stateOptionValues[optionIndex] != optionValues[optionIndex] || (option->flags & OPTION_ALWAYS_SET)) {
        actionTaken = actionOptionToValue(device, option, optionValues[optionIndex], stateOptionValues[optionIndex]);
        optionValues[optionIndex] = stateOptionValues[optionIndex];
    }

    return actionTaken;
}

static int setDeviceOptionToDefault(int deviceIndex, int optionIndex)
{
    const Device* device = activeDevices + deviceIndex;
    const Option* option = ((const Option*) GET_FLASH_PTR(device->optionsOffset)) + optionIndex;
    uint8_t* optionValues = optionValuesStore + deviceDynamicState[deviceIndex].optionValuesOffset;
    int actionTaken = -1;

    if (option->flags & OPTION_DEFAULT_TO_ZERO && optionValues[optionIndex] != 0) {
        if (option->flags & OPTION_ACTION_ON_DEFAULT) {
            actionTaken = actionOptionToValue(device, option, optionValues[optionIndex], 0);
        }

        optionValues[optionIndex] = 0;
    }

    return actionTaken;
}

void deviceSetStatesParallel(const DeviceState* states, int stateCount)
{
    size_t switchingStateSize = sizeof(DeviceSwitchingState) * activeDeviceCount;
    DeviceSwitchingState* deviceSwitchingState = alloca(switchingStateSize);
    memset(deviceSwitchingState, 0, switchingStateSize);

    for (int i = 0; i < activeDeviceCount; i++) {
        deviceSwitchingState[i].newState = getStateForDevice(states, stateCount, activeDevices + i);
    }

    int finishedCount = 0;
    uint32_t lastTime = 0;
    uint32_t currTime = lastTime;

    tpmEnableTimer(0);
    tpmStartTimer(0, TPM_CLOCKS_PER_MILLISECOND, 0);

    while (finishedCount < activeDeviceCount) {
        uint32_t elapsedTime = currTime - lastTime;

        for (int i = 0; i < activeDeviceCount; i++) {
            if (!deviceSwitchingState[i].finished) {
                if (deviceSwitchingState[i].delay) {
#if defined(DELAY_BYPASS)
                    deviceSwitchingState[i].delay = 0;
                    deviceSwitchingState[i].currentOption++;
#else
                    if (deviceSwitchingState[i].delay > elapsedTime) {
                        deviceSwitchingState[i].delay -= elapsedTime;
                    }
                    else {
                        deviceSwitchingState[i].delay = 0;
                        deviceSwitchingState[i].currentOption++;
                    }
#endif
                }
                if (!deviceSwitchingState[i].delay) {
                    if (deviceSwitchingState[i].currentOption >= activeDevices[i].optionCount) {
                        deviceSwitchingState[i].finished = 1;
                        finishedCount++;
                    } else {
                        int actionTaken;

                        if (deviceSwitchingState[i].newState) {
                            actionTaken = setDeviceOptionToState(i, deviceSwitchingState[i].currentOption, deviceSwitchingState[i].newState);
                        } else {
                            actionTaken = setDeviceOptionToDefault(i, deviceSwitchingState[i].currentOption);
                        }

                        if (actionTaken >= 0) {
                            const Device* device = activeDevices + i;
                            const Option* option = ((const Option*) GET_FLASH_PTR(device->optionsOffset)) + deviceSwitchingState[i].currentOption;

                            if (option->postDelaysOffset) {
                                const uint32_t* postDelays = (const uint32_t*) GET_FLASH_PTR(option->postDelaysOffset);

                                if (postDelays[actionTaken] > 0) {
                                    deviceSwitchingState[i].delay = postDelays[actionTaken];
                                }
                            }
                        }

                        if (!deviceSwitchingState[i].delay) {
                            deviceSwitchingState[i].currentOption++;
                        }
                    }
                }
            }
        }

        lastTime = currTime;
        currTime = tpmGetTime(0);
    }

    tpmStopTimer(0);
    tpmDisableTimer(0);
}

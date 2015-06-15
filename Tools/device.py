#=======================================================================
# Copyright Nicholas Tuckett 2015.
# Distributed under the MIT License.
# (See accompanying file license.txt or copy at
#  http://opensource.org/licenses/MIT)
#=======================================================================

import ctypes as ct
from remote import RemoteDataStruct, RemoteDataArray, RemoteDataRefArray
from ir import IrAction

Activity_NoDevices      = 0x0001    # Activity should not use or change state of any devices

Option_Cycled           = 0x0001    # Option cycles through values, otherwise set explicitly to values. 1 action steps up, 2 actions step down/up
Option_DefaultToZero    = 0x0002    # Option is set back to zero if not explicitly set in activity
Option_ActionOnDefault  = 0x0004    # Option requires actions to be generated when returning to default
Option_AlwaysSet        = 0x0008    # Always set the value of this option even if unchanged (e.g. to force to a known state)
Option_AbsoluteFromZero = 0x0010    # For cycled options: when changing, resets to zero and counts up to target. action 1 is reset, action 2 is step

#
# Option - a tracked variable/setting on a device, with a value from 0 to N where 0 < N < 256
#
# C structure:
#   uint16_t    flags;
#   uint8_t     max_value;
#   uint8_t     action_count;
#   offset      pre_action;
#   offset      actions;
#
# The list of actions is interpreted based on the flags and action count:
#   Option_Cycled flag:
#       1 action: advances the option to the next value, wrapping to zero on passing max_value
#       2 action: first decrements the option, second increments the option, wrapping around on 0 or max_value
#   No Option_Cycled flag:
#       One action per value - when a given action occurs, the option is set to the corresponding value
#
class Option(RemoteDataStruct):
    _fields_ = [
        ("flags", ct.c_uint16),
        ("max_value", ct.c_uint8),
        ("action_count", ct.c_uint8),
        ("pre_action", ct.c_uint32),
        ("actions", ct.c_uint32),
        ("post_delays", ct.c_uint32)
        ]

    def __init__(self, name, flags, max_value, change_actions, pre_action, post_delays):
        self.name = name
        self.flags = flags
        self.max_value = max_value
        self.pre_action_ref = pre_action
        self.change_actions = change_actions
        self.action_count = len(self.change_actions)
        self.post_delays_list = post_delays

    def __str__(self):
        return "Option %s" % self.name

    def pre_pack_change_actions(self, package, device):
        self.action_refs = RemoteDataRefArray([x.ref() for x in self.change_actions], self.name + "-actions")
        package.append(self.action_refs)
        
        if self.post_delays_list:
            self.post_delays_ref = RemoteDataArray(self.post_delays_list, ct.c_uint32, self.name + "-postdelays")
            package.append(self.post_delays_ref)
        else:
            self.post_delays_ref = None
        
    def fix_up(self, package):
        if self.pre_action_ref:
            self.pre_action = package.offsetof(self.pre_action_ref.ref())
        self.actions = package.offsetof(self.action_refs.ref())
        
        if self.post_delays_ref:
            self.post_delays = package.offsetof(self.post_delays_ref.ref())

    def binarise(self):
        return ct.string_at(ct.addressof(self), ct.sizeof(self))
                
#
# Device - a physical object that responds to recognised IrActions
#
# C structure:
#   int     option_count;
#   offset  options;        -- offset to contiguous array of options
#
# As each option has a variable-sized array of change actions, 'pre-pack trailing children' is used to pack
# the change action data after all options for the device, so that the option array can be addressed
# contiguously.
#
class Device(RemoteDataStruct):
    _fields_ = [
        ("option_count", ct.c_int),
        ("options", ct.c_uint32)
        ]

    def __init__(self, name = 'unknown'):            
        self.options_list = []
        self.options_lookup = {}
        self.actions = {}
        self.name = name

    def __str__(self):
        return "Device %s" % self.name
        
    def create_action(self, name, codes):
        self.actions[name] = IrAction(codes, self.name + "-" + name)
        
    def create_option(self, name, flags, max_value, change_action_names, pre_action_name = None, post_delays = None):
        if pre_action_name:
            try:
                pre_action = self.actions[pre_action_name]
            except KeyError as e:
                raise RemoteDataError("%s has unrecognised pre-action name %s" % (self, e))
        else:
            pre_action = None

        try:
            change_actions = [self.actions[x] for x in change_action_names]
        except KeyError as e:
            raise RemoteDataError("%s has unrecognised change action name %s" % (self, e))
            
        self.options_lookup[name] = len(self.options_list)
        self.options_list.append(Option(self.name + "-" + name, flags, max_value, change_actions, pre_action, post_delays))
        
    def pre_pack_options_and_actions(self, package):
        for option in self.options_list:
            package.append(option)

        for x in self.options_list:
            x.pre_pack_change_actions(package, self)
            
        for key, value in self.actions.iteritems():
            package.append(value)
            
    def option_index(self, option_name):
        try:
            return self.options_lookup[option_name]
        except KeyError:
            return len(self.options_lookup)

    def pre_pack(self, package):
        self.option_count = len(self.options_list)

    def fix_up(self, package):  
        if len(self.options_list) > 0:
            try:
                self.options = package.offsetof(self.options_list[0].ref())
            except PackageError:
                print self, "has missing options"

#
# DeviceState - represents an expected state of a device in terms of options
#
# C structure:
#   offset      device;
#   offset      option_values;
#
# Size of the option_values array is determined from the device's option count
#
class DeviceState(RemoteDataStruct):
    _fields_ = [
        ("device", ct.c_uint32),
        ("option_values", ct.c_uint32)
    ]
    
    def __init__(self, name, device, option_values_dict):
        self.name = name
        self.device_ref = device
        self.option_values_dict = option_values_dict
        
    def __str__(self):
        return "DeviceState %s" % self.name
    
    def pre_pack_option_values(self, package):
        values = [0] * len(self.device_ref.options_list)
        try:
            for option, value in self.option_values_dict.iteritems():
                values[self.device_ref.option_index(option)] = value
        except IndexError:
            raise PackageError("%s has invalid option in %s" % (self, self.option_values_dict))
            
        self.option_values_ref = RemoteDataArray(values, ct.c_uint8, self.name + "-options")
        package.append(self.option_values_ref)
        
    def fix_up(self, package):        
        try:
            self.device = package.offsetof(self.device_ref.ref())
        except PackageError:
            print self, "has missing device"

        try:
            self.option_values = package.offsetof(self.option_values_ref.ref())
        except PackageError:
            print self, "has missing device"



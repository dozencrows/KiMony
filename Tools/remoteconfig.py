#=======================================================================
# Copyright Nicholas Tuckett 2015.
# Distributed under the MIT License.
# (See accompanying file license.txt or copy at
#  http://opensource.org/licenses/MIT)
#=======================================================================

import ctypes as ct
from remote import RemoteDataStruct
from ui import *
from device import *

#
# Top level structure that pulls together the entire remote data set
#
# C structure:
#   offset  home_activity;
#   int     device_count;
#   offset  devices;            -- contiguous array of devices
#
# The pre-pack trailing children method is used to add the devices to the package,
# so they are grouped into a contiguous array of equal-sized entries - and their
# dynamically sized data is added afterwards
#
class RemoteConfig(RemoteDataStruct):
    _fields_ = [
        ("home_activity", ct.c_uint32),
        ("devices_count", ct.c_int),
        ("devices", ct.c_uint32)
    ]
    
    def __init__(self):
        self.home_activity_ref = None
        self.devices_list = []
        self.activities = []
        self.events = []
        
    def __str__(self):
        return "RemoteConfig"
        
    def set_home_activity(self, home_activity):
        self.home_activity_ref = home_activity
        
    def add_device(self, device):
        self.devices_list.append(device)
        
    def create_event(self, name, type):
        event = Event(type, name = name)
        self.events.append(event)
        return event
        
    def create_activity_event(self, name, activity):
        event = Event(Event_ACTIVITY, [activity], name = name)
        self.events.append(event)
        return event

    def create_ir_event(self, name, device, action):
        event = Event(Event_IRACTION, [device.actions[action], device], name = name)
        self.events.append(event)
        return event

    def add_activity(self, activity):
        self.activities.append(activity)
    
    def pre_pack(self, package):
        for activity in self.activities:
            package.append(activity)
            
        for device in self.devices_list:
            package.append(device)
            
        for event in self.events:
            package.append(event)

    def pre_pack_trailing_children(self, package):
        for device in self.devices_list:
            device.pre_pack_options_and_actions(package)

    def fix_up(self, package):        
        self.devices_count = len(self.devices_list)

        try:
            self.home_activity = package.offsetof(self.home_activity_ref.ref())
        except PackageError:
            print self, "has missing home activity"

        try:
            self.devices = package.offsetof(self.devices_list[0].ref())
        except PackageError:
            print self, "has missing devices"



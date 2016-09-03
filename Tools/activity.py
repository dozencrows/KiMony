#=======================================================================
# Copyright Nicholas Tuckett 2015.
# Distributed under the MIT License.
# (See accompanying file license.txt or copy at
#  http://opensource.org/licenses/MIT)
#=======================================================================

import ctypes as ct
from remote import RemoteDataStruct, RemoteDataArray, RemoteDataRefArray
from ui import ButtonMapping, GestureMapping, TouchButtonPage
from device import DeviceState

#
# Activity - a set of touch screen buttons and physical buttons
#
# C structure:
#       int     button_mapping_count;
#       offset  button_mappings;            -- offset to contiguous array of button mappings
#       int     gesture_mapping_count;
#       offset  gesture_mappings;           -- offset to contiguous array of gesture mappings
#       int     touch_button_page_count;
#       offset  touch_button_pages;         -- offset to contiguous array of button pages
#       int     device_state_count;
#       offset  device_states;              -- offset to contiguous array of device states
#
# The button mappings, gesture mappings, touch button pages and device states arrays will immediately
# follow on from the activity structure in the packed file.
#
# As each touch button page can have a varying number of touch buttons, the 'pre-pack trailing children'
# hook is used to ensure that the touch buttons are packed after the pages. This means that each entry
# in the touch button page array has the same size, so can be addressed contiguously.
#
# Similarly, each device state is of variable size (due to number of options on device) - this is
# packed in the same manner as for touch buttons in touch button pages.
#
class Activity(RemoteDataStruct):
    _fields_ = [
        ("flags", ct.c_uint32),
        ("button_mapping_count", ct.c_int),
        ("button_mappings", ct.c_uint32),
        ("gesture_mapping_count", ct.c_int),
        ("gesture_mappings", ct.c_uint32),
        ("touch_button_page_count", ct.c_int),
        ("touch_button_pages", ct.c_uint32),
        ("device_state_count", ct.c_int),
        ("device_states", ct.c_uint32),
        ]

    def __init__(self, flags = 0, name = 'unknown'):
        self.name = name
        self.flags = flags
        self.button_mapping_objs = []
        self.touch_button_page_objs = []
        self.gesture_mapping_objs = []
        self.device_state_objs = []

    def __str__(self):
        return "Activity %s" % self.name
        
    def create_device_state(self, device, options):
        self.device_state_objs.append(DeviceState(self.name + "-" + device.name, device, options))
        
    def create_button_mapping(self, button_mask, event):
        self.button_mapping_objs.append(ButtonMapping(button_mask, event))

    def create_gesture_mapping(self, gesture, event):
        self.gesture_mapping_objs.append(GestureMapping(gesture, event))

    def create_touch_button_page(self, touch_buttons):
        self.touch_button_page_objs.append(TouchButtonPage(touch_buttons, self.name + "-page-" + str(len(self.touch_button_page_objs) + 1)))
        
    def pre_pack(self, package):
        for x in self.button_mapping_objs:
            package.append(x)
           
        for x in self.gesture_mapping_objs:
            package.append(x)
            
        for x in self.touch_button_page_objs:
            package.append(x)

        for x in self.device_state_objs:
            package.append(x)
 
    def pre_pack_trailing_children(self, package):
        for x in self.touch_button_page_objs:
            x.pre_pack_touch_buttons(package)

        for x in self.device_state_objs:
            x.pre_pack_option_values(package)
            
    def fix_up(self, package):
        self.button_mapping_count = len(self.button_mapping_objs)
        if self.button_mapping_count > 0:
            try:
                self.button_mappings = package.offsetof(self.button_mapping_objs[0].ref())
            except PackageError:
                print self, "has reference to missing button mappings array"

        self.gesture_mapping_count = len(self.gesture_mapping_objs)
        if self.gesture_mapping_count > 0:
            try:
                self.gesture_mappings = package.offsetof(self.gesture_mapping_objs[0].ref())
            except PackageError:
                print self, "has reference to missing gesture mappings array"
        
        self.touch_button_page_count = len(self.touch_button_page_objs)
        if self.touch_button_page_count > 0:
            try:
                self.touch_button_pages = package.offsetof(self.touch_button_page_objs[0].ref())
            except PackageError:
                print self, "has reference to missing touch button pages"

        self.device_state_count = len(self.device_state_objs)
        if self.device_state_count > 0:
            try:
                self.device_states = package.offsetof(self.device_state_objs[0].ref())
            except PackageError:
                print self, "has reference to missing device states"



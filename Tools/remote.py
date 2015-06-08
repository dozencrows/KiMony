#=======================================================================
# Copyright Nicholas Tuckett 2015.
# Distributed under the MIT License.
# (See accompanying file license.txt or copy at
#  http://opensource.org/licenses/MIT)
#=======================================================================

#
# KiMony remote control data structures
#

import ctypes as ct
import struct
import types

IrEncoding_NOP  = 0
IrEncoding_RC6  = 1
IrEncoding_SIRC = 2

Event_NONE      = 0
Event_IRACTION  = 1
Event_ACTIVITY  = 2
Event_NEXTPAGE  = 3
Event_PREVPAGE  = 4
Event_HOME      = 5
Event_DOWNLOAD  = 6
Event_POWEROFF  = 7

Activity_NoDevices      = 0x0001    # Activity should not use or change state of any devices

Option_Cycled           = 0x0001    # Option cycles through values, otherwise set explicitly to values
Option_DefaultToZero    = 0x0002    # Option is set back to zero if not explicitly set in activity
Option_ActionOnDefault  = 0x0004    # Option requires actions to be generated when returning to default

WATERMARK       = 0xBABABEBE

#
# Remote-specific exceptions
#
class PackageError(Exception):
    def __init__(self, message):
        self.message = message
    def __str__(self):
        return self.message
        
class RemoteDataError(Exception):
    def __init__(self, message):
        self.message = message
    def __str__(self):
        return self.message

#
# Base class for all KiMony remote objects
#
class RemoteDataObj(object):
    # Return unique reference, for fixing up to offsets in packaged data
    def ref(self):
        return id(self)
    
    # Hook for per-object preparation prior to packaging - e.g. appending contained
    # objects into the package depth-first
    def pre_pack(self, package):
        pass
        
    # Hook for this object to append child objects in a breadth-first manner
    def pre_pack_trailing_children(self, package):
        pass
        
    # Generate offset values in C structure fields from references to other Python objects
    def fix_up(self, offsets):
        pass

    # Size of C structure
    def size(self):
        return 0
                
    # This object's C structure packed as binary data into a string
    def binarise(self):
        return ''
        
    # Alignment in bytes for start of this object's C structure in packed file
    def alignment(self):
        return 4

#
# Base class for remote objects that are represented as C structures in packed file
#
class RemoteDataStruct(RemoteDataObj, ct.LittleEndianStructure):
    def size(self):
        return ct.sizeof(self)

    def binarise(self):
        return ct.string_at(ct.addressof(self), ct.sizeof(self))

#
# Wrapper for strings, to handle referencing
#
class RemoteDataStr(RemoteDataObj):
    def __init__(self, string):
        self.string = string
        self.wrapped_string = ct.create_string_buffer(string)
        
    def __str__(self):
        return self.string

    def size(self):
        return ct.sizeof(self.wrapped_string)

    def binarise(self):
        return ct.string_at(ct.addressof(self.wrapped_string), ct.sizeof(self.wrapped_string))
        
    def alignment(self):
        return 1

#
# Wrapper for arrays of types that don't require fixup other than copying
#
class RemoteDataArray(RemoteDataObj):
    def __init__(self, values, value_type):
        self.values = values
        self.value_type = value_type
        self.values_array = (value_type * len(values))()

    def __str__(self):
        return "Array %d (type %s, size %d)" % (self.ref(), self.value_type, len(self.values))
        
    def fix_up(self, package):
        self.values_array[:] = [x for x in self.values]

    def size(self):
        return ct.sizeof(self.values_array)
                
    def binarise(self):
        return ct.string_at(ct.addressof(self.values_array), ct.sizeof(self.values_array))

    def alignment(self):
        return ct.sizeof(self.value_type)

# 
# Wrapper for arrays of references
#
class RemoteDataRefArray(RemoteDataArray):
    def __init__(self, values):
        super(RemoteDataRefArray, self).__init__(values, ct.c_uint32)

    def __str__(self):
        return "RefArray %d (size %d)" % (self.ref(), len(self.values))
        
    def fix_up(self, package):
        try:
            self.values_array[:] = [package.offsetof(x) for x in self.values]
        except PackageError:
            print self, "has unsatisfied references"

#
# Single IR code that can be sent by a remote
#
# C structure:
#   uint32_t    encoding:4;     -- type of encoding (SIRC, RC6, ...)
#   uint32_t    bits:5;         -- number of bits in code
#   uint32_t    code:23;        -- code value
#   uint32_t    toggle_mask;    -- mask indicating any toggle bit expected by receiver
#
class IrCode(RemoteDataStruct):
    _fields_ = [
        ("encoding", ct.c_uint32, 4),
        ("bits", ct.c_uint32, 5),
        ("code", ct.c_uint32, 23),
        ("toggle_mask", ct.c_uint32) 
        ]
        
    _encodings_ = { 0:"nop", 1:"RC6", 2:"SIRC" }
        
    def __init__(self, encoding, bits, code, toggle_mask=0):
        self.encoding = encoding
        self.bits = bits
        self.code = code
        self.toggle_mask = toggle_mask
        
    def __repr__(self):
        return self.__str__()
        
    def __str__(self):
        return "IrCode %s %d bits %08x (%08x)" % (IrCode._encodings_[self.encoding], self.bits, self.code, self.toggle_mask)

#
# Single IR 'action' consisting of one or more codes
#
# C structure:
#   int     code_count;
#   IrCode  codes[];
#
# This function dynamically generates a class with the right size of array for the set of codes
# The codes are packaged as part of the structure
#
def IrAction(codes=None, name='unknown'):

    if codes:
        code_count = len(codes)
    else:
        code_count = 1
    
    class IrAction_(RemoteDataStruct):
        _fields_ = [
            ("count", ct.c_int),
            ("codes", IrCode * code_count)
            ]
            
        def __init__(self, name):
            self.name = name
            
        def __str__(self):
            code_list = [self.codes[x] for x in range(0, len(self.codes))]
            return "IrAction %s (%s)" % (self.name, code_list)
            
    o = IrAction_(name)
    o.count = code_count
    
    if codes:
        for x in range(0, o.count):
            o.codes[x] = codes[x]

    return o

#
# KiMony event - e.g. an IrAction, Activity selection
#
# C structure:
#   uint32_t    type;
#   uint32_t    data[2];
#
# Data is ignored for some event types, or treated as an offset to another object for others.
#
class Event(RemoteDataStruct):
    _fields_ = [
        ("type", ct.c_uint32),
        ("data", ct.c_uint32 * 2)
        ]

    _types_ = { 0:"none", 1:"IR action", 2:"Activity", 3:"Next", 4:"Prev", 5:"Home", 6:"Download", 7:"PowerOff" }
        
    def __init__(self, t, data = None, name = 'unknown'):
        self.type = t
        self.data_values = data
        self.name = name
            
    def __str__(self):
        return "Event %s (type %s)" % (self.name, Event._types_[self.type])
    
    def fix_up(self, package):
        if self.data_values and len(self.data_values) > 0:
            if self.type == Event_IRACTION:
                try:
                    self.data[0] = package.offsetof(self.data_values[0].ref())
                    self.data[1] = package.offsetof(self.data_values[1].ref())
                except PackageError:
                    print self, "has reference to missing action or device"
            else:
                try:
                    self.data[0] = package.offsetof(self.data_values[0].ref())
                except PackageError:
                    print self, "has reference to missing object"
                    
#
# Physical button mapping
#
# C structure:
#   uint32_t    button_mask;
#   offset      event;
#
# If pressed button state matches mask, the given event is fired.
#
class ButtonMapping(RemoteDataStruct):
    _fields_ = [
        ("button_mask", ct.c_uint32),
        ("event", ct.c_uint32)
        ]
    
    def __init__(self, button_mask, event):
        self.button_mask = button_mask
        if event:
            self.event_ref = event
        else:
            self.event_ref = None
        
    def __str__(self):
        return "Button Mapping %08x to %s" % (self.button_mask, self.event_ref.name)
        
    def fix_up(self, package):
        if self.event_ref:
            try:
                self.event = package.offsetof(self.event_ref.ref())
            except PackageError:
                print self, "has reference to missing event"

#
# Touch screen button
#
# C structure:
#   offset      event;
#   offset      text;
#   uint16_t    x, y, width, height, colour;
#   uint32_t    press_activate:1;               -- if true, event is fired on press, otherwise release
#   uint32_t    centre_text:1;                  -- if true, text is centered in the button
#
class TouchButton(RemoteDataStruct):
    _fields_ = [
        ("event", ct.c_uint32),
        ("text", ct.c_uint32),
        ("x", ct.c_uint16),
        ("y", ct.c_uint16),
        ("width", ct.c_uint16),
        ("height", ct.c_uint16),
        ("colour", ct.c_uint16),
        ("press_activate", ct.c_uint32, 1),
        ("centre_text", ct.c_uint32, 1),
        ]
        
    def __init__(self, event, text, x, y, width, height, colour, press_activate, centre_text, name = None):
        if text:
            self.wrapped_text = RemoteDataStr(text)
            self.text_ref  = self.wrapped_text.ref()
        else:
            self.wrapped_text = None
            self.text_ref = None
            
        if event:
            self.event_ref = event.ref()
        else:
            self.event_ref = None
            
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.colour = colour
        self.press_activate = press_activate
        self.centre_text = centre_text
        
        if name:
            self.name = name
        elif text:
            self.name = text
        else:
            self.name = 'unknown'
        
    def __str__(self):
        return "TouchButton %s" % self.name
        
    def pre_pack(self, package):
        if self.wrapped_text:
            package.append_text(self.wrapped_text)
        
    def fix_up(self, package):
        try:
            self.event = package.offsetof(self.event_ref)
        except PackageError:
            print self, "has reference to missing event"

        try:
            self.text  = package.offsetof(self.text_ref)
        except PackageError:
            print self, "has reference to missing text"

#
# Page of touch screen buttons
#
# C structure:
#   int     button_count;
#   offset  buttons;
#
# As a page has a variable number of buttons, the button array is kept separate so that multipe pages
# can be packed into one contiguous array using pre_pack_trailing_children().
#
class TouchButtonPage(RemoteDataStruct):
    _fields_ = [
        ("count", ct.c_int),
        ("buttons", ct.c_uint32)
        ]
    
    def __init__(self, touch_buttons, name = 'unknown'):
        self.name = name;
        self.touch_buttons = touch_buttons
        self.count = len(touch_buttons)
        if touch_buttons:
            self.buttons_ref = touch_buttons[0].ref()
        else:
            self.buttons_ref = None
        
    def __str__(self):
        return "TouchButtonPage %s" % self.name
        
    def pre_pack_touch_buttons(self, package):
        for x in self.touch_buttons:
            package.append(x)
            
    def fix_up(self, package):
        try:
            self.buttons = package.offsetof(self.buttons_ref)
        except PackageError:
            print self, "has reference to missing touch buttons array"

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
        ("actions", ct.c_uint32)
        ]

    def __init__(self, name, flags, max_value, change_actions, pre_action):
        self.name = name
        self.flags = flags
        self.max_value = max_value
        self.pre_action_ref = pre_action
        self.change_actions = change_actions
        self.action_count = len(self.change_actions)

    def __str__(self):
        return "Option %s" % self.name

    def pre_pack_change_actions(self, package, device):
        self.action_refs = RemoteDataRefArray([x.ref() for x in self.change_actions])
        package.append(self.action_refs)
        
    def fix_up(self, package):
        if self.pre_action_ref:
            self.pre_action = package.offsetof(self.pre_action_ref.ref())
        self.actions = package.offsetof(self.action_refs.ref())

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
        
    def create_option(self, name, flags, max_value, change_action_names, pre_action_name = None):
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
        self.options_list.append(Option(self.name + "-" + name, flags, max_value, change_actions, pre_action))
        
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
            
        self.option_values_ref = RemoteDataArray(values, ct.c_uint8)
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

#
# Activity - a set of touch screen buttons and physical buttons
#
# C structure:
#       int     button_mapping_count;
#       offset  button_mappings;            -- offset to contiguous array of button mappings
#       int     touch_button_page_count;
#       offset  touch_button_pages;         -- offset to contiguous array of button pages
#       int     device_state_count;
#       offset  device_states;              -- offset to contiguous array of device states
#
# The button mappings, touch button pages and device states arrays will immediately follow on from the
# activity structure in the packed file.
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
        self.device_state_objs = []

    def __str__(self):
        return "Activity %s" % self.name
        
    def create_device_state(self, device, options):
        self.device_state_objs.append(DeviceState(self.name + "-" + device.name, device, options))
        
    def create_button_mapping(self, button_mask, event):
        self.button_mapping_objs.append(ButtonMapping(button_mask, event))

    def create_touch_button_page(self, touch_buttons):
        self.touch_button_page_objs.append(TouchButtonPage(touch_buttons, self.name + "-page-" + str(len(self.touch_button_page_objs) + 1)))
        
    def pre_pack(self, package):
        for x in self.button_mapping_objs:
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

#
# Class used to bundle up and encode KiMony remote data objects into binary
# for squirting down to the device
#       
class Package:
    def __init__(self):
        self.offsets = {}
        self.objects = []
        self.texts = []
        self.next_offset = 0
        self.text_offset = 0
        self.errors = 0
        
    def append(self, obj):
        self.align_to(obj.alignment())
        self.offsets[obj.ref()] = self.next_offset
        self.objects.append(obj)
        self.next_offset += obj.size()
        obj.pre_pack(self)
        obj.pre_pack_trailing_children(self)
        
    def append_text(self, text):
        self.offsets[text.ref()] = self.text_offset
        self.texts.append(text)
        self.text_offset += text.size()
        
    def pack(self):
        packed_objects = [ struct.pack("<I", WATERMARK) ]
        packed_offset = 0
        
        for text in self.texts:
            self.offsets[text.ref()] += self.next_offset
        
        all_objs = self.objects + self.texts
        
        for obj in all_objs:
            obj.fix_up(self)

            fill_size = self.offsets[obj.ref()] - packed_offset
            packed_objects.append('\x00' * fill_size)

            binary_obj = obj.binarise()
            packed_objects.append(obj.binarise())

            print "Obj", obj, "at", self.offsets[obj.ref()], "size", len(binary_obj), "fill", fill_size
            packed_offset += len(binary_obj) + fill_size

        self.align_to(4)
        fill_size = self.next_offset - packed_offset
        packed_objects.append('\x00' * fill_size)

        if self.errors:
            raise PackageError("Errors during packing")
                                
        return ''.join(packed_objects)
        
    def offsetof(self, ref):
        try:
            if ref:
                return self.offsets[ref]
            else:
                return 0
        except KeyError:
            self.errors += 1
            raise PackageError('Missing object %d' % ref)
        
    def align_to(self, alignment):
        misalignment = self.next_offset % alignment
        self.next_offset += (alignment - misalignment) % alignment
        

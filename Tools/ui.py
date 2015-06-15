#=======================================================================
# Copyright Nicholas Tuckett 2015.
# Distributed under the MIT License.
# (See accompanying file license.txt or copy at
#  http://opensource.org/licenses/MIT)
#=======================================================================

import ctypes as ct
from remote import RemoteDataStruct, RemoteDataStr, RemoteDataError
from image import RemoteImage
from ir import IrAction

Event_NONE      = 0
Event_IRACTION  = 1
Event_ACTIVITY  = 2
Event_NEXTPAGE  = 3
Event_PREVPAGE  = 4
Event_HOME      = 5
Event_DOWNLOAD  = 6
Event_POWEROFF  = 7

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
#   uint16_t    x, y, width, height, colour, flags;
#   offset      image1, image2;
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
        ("flags", ct.c_uint16),
        ("image1", ct.c_uint32),
        ("image2", ct.c_uint32),
        ]

    FLAGS_PRESS_ACTIVATE = 0x0001
    FLAGS_CENTRE_TEXT    = 0x0002
    FLAGS_NO_BORDER      = 0x0004
    FLAGS_NO_FILL        = 0x0008
        
    def __init__(self, event, text, x, y, width, height, colour, flags = 0, image1 = None, image2 = None, name = None):
        if name:
            self.name = name
        elif text:
            self.name = text
        elif event.name:
            self.name = event.name
        else:
            self.name = 'unknown'

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
        
        try:    
            if image1:
                self.image1_obj = RemoteImage.from_path(image1)
                self.image1_ref = self.image1_obj.ref()
            else:
                self.image1_ref = None

            if image2:
                self.image2_obj = RemoteImage.from_path(image2)
                self.image2_ref = self.image2_obj.ref()
            else:
                self.image2_ref = None
        except IOError as e:
            raise RemoteDataError("%s has problem with image: %s" % (self, e))
                        
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.colour = colour
        self.flags = flags
        
    def __str__(self):
        return "TouchButton %s" % self.name
        
    def pre_pack(self, package):
        if self.wrapped_text:
            package.append_text(self.wrapped_text)
            
    def pre_pack_images(self, package):
        if self.image1_ref:
            package.append(self.image1_obj)
        if self.image2_ref:
            package.append(self.image2_obj)
        
    def fix_up(self, package):
        try:
            self.event = package.offsetof(self.event_ref)
        except PackageError:
            print self, "has reference to missing event"

        try:
            self.text  = package.offsetof(self.text_ref)
        except PackageError:
            print self, "has reference to missing text"
            
        if self.image1_ref:
            self.image1 = package.offsetof(self.image1_ref)

        if self.image2_ref:
            self.image2 = package.offsetof(self.image2_ref)

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
            
        for x in self.touch_buttons:
            x.pre_pack_images(package)
            
    def fix_up(self, package):
        try:
            self.buttons = package.offsetof(self.buttons_ref)
        except PackageError:
            print self, "has reference to missing touch buttons array"


#
# KiMony structures:
#

import ctypes as ct
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

#
# Base class for all KiMony remote objects
#
class RemoteDataObj(ct.LittleEndianStructure):
    def ref(self):
        return id(self) & 0xffffffff
        
    def pre_pack(self, package):
        pass
        
    def fix_up(self, offsets):
        pass

    def size(self):
        return ct.sizeof(self)
                
    def binarise(self):
        return ct.string_at(ct.addressof(self), ct.sizeof(self))
        
    def alignment(self):
        return 4

#
# Wrapper for strings, to handle referencing
#
class RemoteDataStr(RemoteDataObj):
    def __init__(self, string):
        self.wrapped_string = ct.create_string_buffer(string)

    def size(self):
        return ct.sizeof(self.wrapped_string)

    def binarise(self):
        return ct.string_at(ct.addressof(self.wrapped_string), ct.sizeof(self.wrapped_string))
        
    def alignment(self):
        return 1
 
#
# Single IR code that can be sent by a remote
#           
class IrCode(RemoteDataObj):
    _fields_ = [
        ("encoding", ct.c_uint32, 4),
        ("bits", ct.c_uint32, 5),
        ("code", ct.c_uint32, 23),
        ("toggle_mask", ct.c_uint32) 
        ]
        
    def __init__(self, encoding, bits, code, toggle_mask=0):
        self.encoding = encoding
        self.bits = bits
        self.code = code
        self.toggle_mask = toggle_mask

#
# Single IR 'action' consisting of one or more codes
#
def IrAction(codes=None):
    if codes:
        code_count = len(codes)
    else:
        code_count = 1
    
    class IrAction_(RemoteDataObj):
        _fields_ = [
            ("count", ct.c_int),
            ("codes", IrCode * code_count)
            ]
    o = IrAction_()
    o.count = code_count
    
    if codes:
        for x in range(0, o.count):
            o.codes[x] = codes[x]

    return o

#
# KiMony event - e.g. an IrAction, Activity selection
#
class Event(RemoteDataObj):
    _fields_ = [
        ("type", ct.c_uint32),
        ("data", ct.c_uint32)
        ]
        
    def __init__(self, t, data):
        self.type = t
        if data:
            self.data = data.ref()
    
    def fix_up(self, package):
        if self.data:
            self.data = package.offsetof(self.data)

#
# Physical button mapping
#
class ButtonMapping(RemoteDataObj):
    _fields_ = [
        ("button_mask", ct.c_uint32),
        ("event", ct.c_uint32)
        ]
    
    def __init__(self, button_mask, event):
        self.button_mask = button_mask
        self.event = event.ref()
        
    def fix_up(self, package):
        self.event = package.offsetof(self.event)

#
# Touch screen button
#
class TouchButton(RemoteDataObj):
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
        
    def __init__(self, event, text, x, y, width, height, colour, press_activate, centre_text):
        if text:
            self.wrapped_text = RemoteDataStr(text)
            self.text  = self.wrapped_text.ref()
        else:
            self.wrapped_text = None
        if event:
            self.event = event.ref()
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.colour = colour
        self.press_activate = press_activate
        self.centre_text = centre_text
        
    def pre_pack(self, package):
        if self.wrapped_text:
            package.append_text(self.wrapped_text)
        
    def fix_up(self, package):
        self.event = package.offsetof(self.event)
        self.text  = package.offsetof(self.text)

#
# Page of touch screen buttons
#
class TouchButtonPage(RemoteDataObj):
    _fields_ = [
        ("count", ct.c_int),
        ("buttons", ct.c_uint32)
        ]
    
    def __init__(self, touch_buttons):
        self.touch_buttons = touch_buttons
        self.count = len(touch_buttons)
        self.buttons = touch_buttons[0].ref()
        
    def pre_pack(self, package):
        for x in self.touch_buttons:
            package.append(x)
            
    def fix_up(self, package):
        self.buttons = package.offsetof(self.buttons)

#
# Activity - a set of touch screen buttons and physical buttons
#
class Activity(RemoteDataObj):
    _fields_ = [
        ("button_mapping_count", ct.c_int),
        ("button_mappings", ct.c_uint32),
        ("touch_button_page_count", ct.c_int),
        ("touch_button_pages", ct.c_uint32),
        ]

    def __init__(self, button_mappings, touch_button_pages):
        if button_mappings:
            self.button_mapping_count = len(button_mappings)
            self.button_mappings = button_mappings[0].ref()
            self.button_mapping_objs = button_mappings
        else:
            self.button_mapping_objs = []

        if touch_button_pages:
            self.touch_button_page_count = len(touch_button_pages)
            self.touch_button_pages = touch_button_pages[0].ref()
            self.touch_button_pages_objs = touch_button_pages
        else:
            self.touch_button_pages_objs = []
            
        
    def pre_pack(self, package):
        for x in self.button_mapping_objs:
            package.append(x)
            
        for x in self.touch_button_pages_objs:
            package.append(x)
            
    def fix_up(self, package):
        self.button_mappings = package.offsetof(self.button_mappings)
        self.touch_button_pages = package.offsetof(self.touch_button_pages)

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
        
    def append(self, obj):
        self.align_to(obj.alignment())
        self.offsets[obj.ref()] = self.next_offset
        self.objects.append(obj)
        self.next_offset += obj.size()
        obj.pre_pack(self)
        
    def append_text(self, text):
        self.offsets[text.ref()] = self.text_offset
        self.texts.append(text)
        self.text_offset += text.size()
        
    def pack(self):
        packed_objects = []
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

            print "Obj", obj.ref(), "(", type(obj), ") at", self.offsets[obj.ref()], "size", len(binary_obj), "fill", fill_size
            packed_offset += len(binary_obj) + fill_size

        self.align_to(4)
        fill_size = self.next_offset - packed_offset
        packed_objects.append('\x00' * fill_size)
                    
        return ''.join(packed_objects)
        
    def offsetof(self, ref):
        if ref:
            return self.offsets[ref]
        else:
            return 0
        
    def align_to(self, alignment):
        misalignment = self.next_offset % alignment
        self.next_offset += (alignment - misalignment) % alignment
        

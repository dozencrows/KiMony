#!/usr/bin/python
#
# Tool for updating KiMony data over USB serial
#

#
# KiMony structures:
#   IrCode: 
#       bitfield    encoding:4 bits:5 code:23
#       uint32_t    toggleMask
#
#   IrAction:
#       int         code count
#       IrCode[]    codes
#
#   Event:
#       uint32_t    type:   0 (none), 1 (IrAction), 2 (Activity), 3 (NextPage), 4 (PrevPage), 5 (Home)
#       union       IrAction* irAction, Activity* activity
#
#   ButtonMapping:
#       uint32_t    buttonMask
#       Event*      event
#
#   TouchButton:
#       Event*      event
#       char*       text
#       uint16_t    x, y
#       uint16_t    width, height
#       uint16_t    colour
#       uint8_t     flags           bit 0: press activates, bit 1: centre text
#
#   TouchButtonPage:
#       int             count
#       TouchButtons*   buttons
#
#   Activity:
#       int                 buttonMappingCount
#       ButtonMappng*       buttonMappings
#       int                 touchButtonPageCount
#       TouchButtonPage*    touchButtonPages

# Key aspects:
#   bitfield packing
#   aligning
#   using offsets instead of pointers

import config
import serial
import struct
import time
import sys

ser = serial.Serial(
    port='/dev/ttyUSB0',
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

packed_data_words = len(config.packed_data) / 4
data = struct.pack("<bh", 0x10, packed_data_words)
#print ':'.join(x.encode('hex') for x in data)
ser.write(data)
response = ord(ser.read(1))
print hex(response)
if response == 0x10:
    ser.write(config.packed_data)

    response = ser.read(1)
    print hex(ord(response))
    time.sleep(1)

    data = struct.pack("<bh", 0x20, packed_data_words)
    ser.write(data)
    ser.write(config.packed_data)

ser.close()

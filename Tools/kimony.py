#!/usr/bin/python
#
# Tool for updating KiMony data over USB serial
#

import serial
import struct
import time
import array
import random

ser = serial.Serial(
    port='/dev/ttyUSB0',
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

flashData = array.array('b', [random.randint(-127, 127) for x in range(0,1024)])

data = struct.pack("<bh", 0x10, 0x100)
ser.write(data)
response = ord(ser.read(1))
print hex(response)
if response == 0x10:
    ser.write(flashData.tostring())

    response = ser.read(1)
    print hex(ord(response))
    time.sleep(1)

    data = struct.pack("<bh1024s", 0x20, 0x100, flashData.tostring())
    ser.write(data)

ser.close()


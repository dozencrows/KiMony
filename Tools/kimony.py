#!/usr/bin/python
#
# Tool for updating KiMony data over USB serial
#
import argparse
import traceback

parser = argparse.ArgumentParser(description="Compile data for KiMony programmable remote")
parser.add_argument("output", help="file or device")
parser.add_argument("-s", "--save", action="store_true", help="save config binary data to file")
parser.add_argument("-d", "--download", action="store_true", help="download config binary data to device")
parser.add_argument("-v", "--verbose", action="store_true", help="verbose output for debugging")

args = parser.parse_args()

import serial
import struct
import time
import sys

def download(device):
    packed_data_words = len(config.packed_data) / 4
    
    ser = serial.Serial(
        port=device,
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
    )

    data = struct.pack("<bh", 0x10, packed_data_words)
    #print ':'.join(x.encode('hex') for x in data)
    ser.write(data)
    response = ord(ser.read(1))

    if response == 0x10:
        ser.write(config.packed_data)

        response = ser.read(1)
        print "Validating..."
        time.sleep(1)

        data = struct.pack("<bh", 0x20, packed_data_words)
        ser.write(data)
        ser.write(config.packed_data)
        response = ord(ser.read(1))
        if response == 1:
            print "Ok"
        else:
            print "Validation failed - errors found"
    else:
        print "Error downloading to device"

    ser.close()

def save(path):
    f = open(path, "wb")
    f.write(config.packed_data)
    f.close()

try:
    import config

    config.packed_data = config.package.pack()
    if args.download:
        download(args.output)
    elif args.save:
        save(args.output)
except Exception as e:
    if args.verbose:
        traceback.print_exc()
    else:
        print e


#=======================================================================
# Copyright Nicholas Tuckett 2015.
# Distributed under the MIT License.
# (See accompanying file license.txt or copy at
#  http://opensource.org/licenses/MIT)
#=======================================================================

import ctypes as ct
import struct
import types

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
    def fix_up(self, package):
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
    def __init__(self, values, value_type, name = 'unknown'):
        self.values = values
        self.value_type = value_type
        self.values_array = (value_type * len(values))()
        self.name = name

    def __str__(self):
        return "Array %s (type %s, size %d)" % (self.name, self.value_type, len(self.values))
        
    def fix_up(self, package):
        self.values_array[:] = [x for x in self.values]

    def size(self):
        return ct.sizeof(self.values_array)
                
    def binarise(self):
        return ct.string_at(ct.addressof(self.values_array), ct.sizeof(self.values_array))

    def alignment(self):
        return ct.sizeof(self.value_type)

#
# Wrapper for arrays of data already in binary form
#
class RemoteDataBinaryArray(RemoteDataObj):
    def __init__(self, data, name = 'unknown', alignment = 4):
        self.data = data
        self.name = name
        self.aligned_size = alignment

    def __str__(self):
        return "Binary Array %s (size %d)" % (self.name, len(self.data))
        
    def fix_up(self, package):
        pass

    def size(self):
        return len(self.data)
                
    def binarise(self):
        return self.data

    def alignment(self):
        return self.aligned_size


# from_buffer_copy
# 
# Wrapper for arrays of references
#
class RemoteDataRefArray(RemoteDataArray):
    def __init__(self, values, name = 'unknown'):
        super(RemoteDataRefArray, self).__init__(values, ct.c_uint32, name)

    def __str__(self):
        return "RefArray %s (size %d)" % (self.name, len(self.values))
        
    def fix_up(self, package):
        try:
            self.values_array[:] = [package.offsetof(x) for x in self.values]
        except PackageError:
            print self, "has unsatisfied references"

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
        

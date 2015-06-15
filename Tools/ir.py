#=======================================================================
# Copyright Nicholas Tuckett 2015.
# Distributed under the MIT License.
# (See accompanying file license.txt or copy at
#  http://opensource.org/licenses/MIT)
#=======================================================================

import ctypes as ct
from remote import RemoteDataStruct

IrEncoding_NOP  = 0
IrEncoding_RC6  = 1
IrEncoding_SIRC = 2

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



#
# Image module
#

import ctypes as ct
import os.path

from PIL import Image
from remote import RemoteDataStruct
from remote import RemoteDataArray

BLACK              = (0,   0,   0)
TRANSPARENT_COLOUR = (255, 0, 255)
PALETTE_SIZE       = 256

class RemoteImage(RemoteDataStruct):
    _instances_ = {}

    _fields_ = [
        ("width", ct.c_uint16),
        ("height", ct.c_uint16),
        ("palette", ct.c_uint32),       # store as RemoteDataArray
        ("pixels", ct.c_uint32)         # store as RemoteDataArray
        ]

    #
    # Load an image, convert it to 256 colour form with colour 0 as
    # transparent (taken from magenta)
    #
    @staticmethod
    def __load_image__(path):
        i = Image.open(path)
        p = i.quantize(colors=254).getpalette()
        p = zip(p[0::3], p[1::3], p[2::3])
        try:
            transparent_index = p.index(TRANSPARENT_COLOUR)
            p[0], p[transparent_index] = p[transparent_index], p[0]
        except ValueError:
            p[PALETTE_SIZE - 1] = TRANSPARENT_COLOUR
            p[0], p[PALETTE_SIZE - 1] = p[PALETTE_SIZE - 1], p[0]
            
        palette_image = Image.new('P', i.size)
        palette_image.putpalette([x for y in p for x in y])
        
        return i.quantize(palette=palette_image)

    
    def __init__(self, path):
        self.path = path
        self.name = os.path.basename(path)
        
        self.image_data = RemoteImage.__load_image__(path)
            
        self.width = self.image_data.size[0]
        self.height = self.image_data.size[1]
        
        self._instances_[path] = self
        
    def __str__(self):
        return "Image %s (size %s)" % (self.name, self.image_data.size)
    
    #
    # Factory method to create or look up existing instance, given path
    #
    @classmethod
    def from_path(cls, path):
        rpath = os.path.realpath(os.path.normcase(path))
        try:
            return cls._instances_[rpath]
        except KeyError:
            return cls(rpath)
    
    def pre_pack(self, package):
        # create array for palette and append to package, storing ref. Use list form of palette, converted to 16-bit 565 values
        # create array for pixel data and append to package, storing ref. Used binarised form of pixels, from self.image_data.tobytes() (may not work)
        pass
        
    def fix_up(self, offsets):
        # Fix up palette and pixel data references
        pass


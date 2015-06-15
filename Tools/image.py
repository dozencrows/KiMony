#=======================================================================
# Copyright Nicholas Tuckett 2015.
# Distributed under the MIT License.
# (See accompanying file license.txt or copy at
#  http://opensource.org/licenses/MIT)
#=======================================================================
#
# Image module
#

import ctypes as ct
import os.path
from PIL import Image

from remote import RemoteDataStruct, RemoteDataArray, RemoteDataBinaryArray

BLACK              = (0,   0,   0)
TRANSPARENT_COLOUR = (255, 0, 255)
PALETTE_SIZE       = 256
    
class RemoteImage(RemoteDataStruct):
    _instances_ = {}

    _fields_ = [
        ("width", ct.c_uint16),
        ("height", ct.c_uint16),
        ("palette", ct.c_uint32),
        ("pixels", ct.c_uint32)
        ]

    #
    # Load an image, convert it to 256 colour form with colour 0 as
    # transparent (taken from magenta)
    #
    @staticmethod
    def __load_image(path):
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

    @staticmethod
    def __get_palette_rgb_565(rgb_palette):
        p = zip(rgb_palette[0::3], rgb_palette[1::3], rgb_palette[2::3])
        return [((x[0] & 0xf8) << 8)|((x[1] & 0xf8) << 3)|((x[2]) >> 3) for x in p]
    
    def __init__(self, path):
        self.path = path
        self.name = os.path.basename(path)
        
        self.image_data = RemoteImage.__load_image(path)
            
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
        rgb565_palette = RemoteImage.__get_palette_rgb_565(self.image_data.getpalette())
        self.palette_ref = RemoteDataArray(rgb565_palette, ct.c_uint16, self.name + "-palette")
        package.append(self.palette_ref)
        self.pixels_ref = RemoteDataBinaryArray(self.image_data.tobytes(), self.name + "-pixels")
        package.append(self.pixels_ref)
        
    def fix_up(self, package):
        self.palette = package.offsetof(self.palette_ref.ref())
        self.pixels = package.offsetof(self.pixels_ref.ref())


# Copyright (c) 2024-2025 Hussain Al Marzooq

from .models import models, aliases
import ctypes
import os

#renaming things to make the code less ugly
unsigned_char = ctypes.c_ubyte
char = ctypes.c_byte
unsigned_short = ctypes.c_ushort
short = ctypes.c_short
size_t = ctypes.c_size_t
word_t = ctypes.c_ulonglong
void_p = ctypes.c_void_p
pointer = ctypes.POINTER

class model_t(ctypes.Structure):
    _fields_ = [('width', unsigned_short),
                ('cycle', short),
                ('back', short),
                ('ref', char),
                ('rev', char),
                ('poly', word_t),
                ('init', word_t),
                ('xorout', word_t),
                ('table', pointer(word_t)),
                ('table_comb', pointer(word_t))]

crcany = ctypes.CDLL(os.path.join(os.path.dirname(__file__), 'crcany.dll'))

#model.h
crcany.get_model.argtypes = [unsigned_short, word_t, word_t, char, char, word_t]
crcany.get_model.restype = model_t

crcany.init_model.argtypes = [pointer(model_t)]
crcany.init_model.restype = char

crcany.free_model.argtypes = [pointer(model_t)]

#----------

#crc.h
crcany.crc_table_bytewise.argtypes = [pointer(model_t)]
crcany.crc_bytewise.argtypes = [pointer(model_t), word_t, void_p, size_t]
crcany.crc_bytewise.restype = word_t

crcany.crc_table_slice16.argtypes = [pointer(model_t)]
crcany.crc_slice16.argtypes = [pointer(model_t), word_t, void_p, size_t]
crcany.crc_slice16.restype = word_t

crcany.crc_table_combine.argtypes = [pointer(model_t)]
crcany.crc_combine.argtypes = [pointer(model_t), word_t, word_t, word_t]
crcany.crc_combine.restype = word_t

#----------

try:
    from bitarray import bitarray, frozenbitarray
    ba_imported = True
except ModuleNotFoundError:
    ba_imported = False

cache = {}

class _Crc:
    def __init__(self, width, poly, init, refin, refout, xorout, check=None):
        names = ('width', 'poly', 'init', 'refin', 'refout', 'xorout')
        values = (width, poly, init, refin, refout, xorout)

        for name, value in zip(names, values):
            if value is None:
                raise ValueError(f'{name} value is not provided')

        if width > 64:
            raise ValueError(f'width is larger than 64 bits')

        self.model = crcany.get_model(width, poly, init, refin, refout, xorout)
        error_code = crcany.init_model(ctypes.byref(self.model))

        if error_code == 1:
            raise MemoryError('Out of memory error')

        crcany.crc_table_bytewise(ctypes.byref(self.model))
        crcany.crc_table_slice16(ctypes.byref(self.model))
        crcany.crc_table_combine(ctypes.byref(self.model))

    def __del__(self):
        crcany.free_model(ctypes.byref(self.model))

    def calc(self, data, init=None):
        if ba_imported and (isinstance(data, bitarray) or isinstance(data, frozenbitarray)):
            raise TypeError('Bitarray objects are not allowed, use calc_bits() instead')

        if isinstance(data, str):
            data = data.encode('utf-8')

        if init is None:
            init = self.model.init

        if len(data) == 0:
            return init

        #this seems to be the way to access the buffer protocol from ctypes
        with memoryview(data) as view:
            buf = (unsigned_char * view.nbytes).from_buffer_copy(view)
            return crcany.crc_slice16(ctypes.byref(self.model), init, buf, view.nbytes * 8)

    def calc_bits(self, data, init=None):
        if not ba_imported:
            raise ModuleNotFoundError('The bitarray module is required')

        if not isinstance(data, bitarray) and not isinstance(data, frozenbitarray):
            raise TypeError('Expected a bitarray object')

        if self.model.ref and data.endian() != 'little':
            raise ValueError('A little endian bitarray object is expected for reflected CRCs')

        if not self.model.ref and data.endian() != 'big':
            raise ValueError('A big endian bitarray object is expected for non-reflected CRCs')

        if init is None:
            init = self.model.init

        if len(data) == 0:
            return init

        with memoryview(data) as view:
            buf = (unsigned_char * view.nbytes).from_buffer_copy(view)
            return crcany.crc_slice16(ctypes.byref(self.model), init, buf, len(data))

    def combine(self, crc1, crc2, length):
        return crcany.crc_combine(ctypes.byref(self.model), crc1, crc2, length * 8)

    def combine_bits(self, crc1, crc2, length):
        return crcany.crc_combine(ctypes.byref(self.model), crc1, crc2, length)

    #byte-by-byte (for testing)
    def _calc_b(self, data):
        return crcany.crc_bytewise(ctypes.byref(self.model), self.model.init, data, len(data) * 8)

def CRC(width=None, poly=None, init=None, refin=None, refout=None, xorout=None, check=None):
    values = (width, poly, init, refin, refout, xorout)

    if values in cache:
        return cache[values]
    else:
        crc = _Crc(*values)
        cache[values] = crc
        return crc

def Model(name):
    if name in models:
        return CRC(*models[name])
    elif name in aliases:
        return CRC(*models[aliases[name]])
    else:
        raise ValueError('CRC model not found')
# Copyright (c) 2024 Hussain Al Marzooq

from libc.stdint cimport uintmax_t, intmax_t
from .models import *

import bitarray

ctypedef uintmax_t word_t

cdef extern from '../../lib/crcany/model.h':
    cdef const unsigned short WORDBITS

    ctypedef struct model_t:
        unsigned short width
        char ref, rev
        word_t poly, poly_hi
        word_t init, init_hi
        word_t xorout, xorout_hi
        word_t check, check_hi
        word_t *table_byte
        word_t *table_slice16

    cdef model_t get_model(unsigned short width, word_t poly, word_t init, char refin, char refout, word_t xorout, word_t check)
    cdef void free_model(model_t *model)

cdef extern from '../../lib/crcany/crc.h':
    cdef word_t crc_bitwise(model_t *model, word_t crc, const void *dat, size_t len);

    cdef int crc_table_bytewise(model_t *model)
    cdef word_t crc_bytewise(model_t *model, word_t crc, const void *dat, size_t len);

    cdef int crc_table_slice16(model_t *model)
    cdef word_t crc_slice16(model_t *model, word_t crc, const void *dat, size_t len)

cdef word_t MASK = -1
word_bits = WORDBITS #accessible from python

cdef class CRC:
    cdef model_t model
    cdef word_t register

    def __cinit__(self, width=None, poly=None, init=None, refin=None, refout=None, xorout=None, check=0):
        if width is None:
            raise ValueError('width value is not provided')
        elif width <= 0:
            raise ValueError('width should be larger than zero')
        elif width > word_bits:
            raise ValueError(f'width is larger than {word_bits} bits')

        if poly is None:
            raise ValueError('poly value is not provided')
        elif poly < 0:
            raise ValueError('poly should be a positive value')

        if init is None:
            raise ValueError('init value is not provided')
        elif init < 0:
            raise ValueError('init should be a positive value')

        if refin is None:
            raise ValueError('refin value is not provided')

        if refout is None:
            raise ValueError('refout value is not provided')

        if xorout is None:
            raise ValueError('xorout value is not provided')
        elif xorout < 0:
            raise ValueError('xorout should be a positive value')

        cdef int error_code

        self.model = get_model(width, poly, init, refin, refout, xorout, check)
        error_code = crc_table_bytewise(&self.model)

        if error_code == 1:
            raise MemoryError('Out of memory error')

        error_code = crc_table_slice16(&self.model)

        if error_code == 1:
            raise MemoryError('Out of memory error')

        self.register = self.model.init

    def __dealloc__(self):
        free_model(&self.model);

    def get(self):
        return self.register

    def set(self, crc):
        self.register = crc

    def reset(self):
        self.register = self.model.init

    def calc(self, data, intmax_t length=-1):
        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')

        elif isinstance(data, bitarray.bitarray) or isinstance(data, bitarray.frozenbitarray):
            #clear the pad bits so that they won't affect the CRC calculation
            v = memoryview(data)
            v[-1] &= MASK << data.padbits
            length = len(data)

        cdef const unsigned char[:] view = data
        cdef intmax_t size = len(view) * 8

        if length < 0:
            length = size
        elif length > size:
            raise ValueError('length is larger than the data\'s size')

        if self.model.ref and length > 0 and length % 8 > 0:
            raise ValueError('bit lengths are not supported with reflected CRCs')

        return crc_slice16(&self.model, self.register, &view[0], length)

    def update(self, data, intmax_t length=-1):
        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')

        elif isinstance(data, bitarray.bitarray) or isinstance(data, bitarray.frozenbitarray):
            #clear the pad bits so that they won't affect the CRC calculation
            v = memoryview(data)
            v[-1] &= MASK << data.padbits
            length = len(data)

        cdef const unsigned char[:] view = data
        cdef intmax_t size = len(view) * 8

        if length < 0:
            length = size
        elif length > size:
            raise ValueError('length is larger than the data\'s size')

        if self.model.ref and length > 0 and length % 8 > 0:
            raise ValueError('bit lengths are not supported with reflected CRCs')

        self.register = crc_slice16(&self.model, self.register, &view[0], length)
        return self.register

    #byte-by-byte (for testing)
    def _calc_b(self, data, intmax_t length=-1):
        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')

        elif isinstance(data, bitarray.bitarray) or isinstance(data, bitarray.frozenbitarray):
            #clear the pad bits so that they won't affect the CRC calculation
            v = memoryview(data)
            v[-1] &= MASK << data.padbits
            length = len(data)

        cdef const unsigned char[:] view = data
        cdef intmax_t size = len(view) * 8

        if length < 0:
            length = size
        elif length > size:
            raise ValueError('length is larger than the data\'s size')

        if self.model.ref and length > 0 and length % 8 > 0:
            raise ValueError('bit lengths are not supported with reflected CRCs')

        return crc_bytewise(&self.model, self.register, &view[0], length)

def Model(name):
    if name in models:
        return CRC(*models[name])
    elif name in aliases:
        return CRC(*models[aliases[name]])
    else:
        raise ValueError('CRC model not found')
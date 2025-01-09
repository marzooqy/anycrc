# Copyright (c) 2024-2025 Hussain Al Marzooq

from libc.stdint cimport uintmax_t
from .models import models, aliases
import sys

cdef extern from '../../lib/crcany/model.h':
    ctypedef unsigned int word_t
    cdef const unsigned short WORDBITS

    ctypedef struct model_t:
        unsigned short width
        short cycle, back
        char ref, rev
        word_t poly, init, xorout
        word_t *table
        word_t *table_comb

    cdef model_t get_model(unsigned short width, word_t poly, word_t init, char refin, char refout, word_t xorout)
    cdef char init_model(model_t *model)
    cdef void free_model(model_t *model)

cdef extern from '../../lib/crcany/crc.h':
    cdef void crc_table_bytewise(model_t *model)
    cdef word_t crc_bytewise(model_t *model, word_t crc, const void *dat, size_t len);

    cdef void crc_table_slice16(model_t *model)
    cdef word_t crc_slice16(model_t *model, word_t crc, const void *dat, size_t len)

    cdef void crc_table_combine(model_t *model)
    word_t crc_combine(model_t *model, word_t crc1, word_t crc2, uintmax_t len2)

cache = {}

cdef class _Crc:
    cdef model_t model

    def __cinit__(self, width, poly, init, refin, refout, xorout, check=None):
        self.model = get_model(width, poly, init, refin, refout, xorout)
        cdef char error_code = init_model(&self.model)

        if error_code == 1:
            raise MemoryError('Out of memory error')

        crc_table_bytewise(&self.model)
        crc_table_slice16(&self.model)
        crc_table_combine(&self.model)

    def __dealloc__(self):
        free_model(&self.model);

    def calc(self, data, init=None):
        if 'bitarray' in sys.modules and (isinstance(data, sys.modules['bitarray'].bitarray) or isinstance(data, sys.modules['bitarray'].frozenbitarray)):
            raise TypeError('Bitarray objects are not allowed, use calc_bits() or update_bits() instead')

        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')

        if init is None:
            init = self.model.init

        cdef const unsigned char[:] view = data
        return crc_slice16(&self.model, init, &view[0], len(view) * 8)

    def calc_bits(self, data, init=None):
        if 'bitarray' not in sys.modules:
            raise ModuleNotFoundError('The bitarray module is required')

        if not isinstance(data, sys.modules['bitarray'].bitarray) and not isinstance(data, sys.modules['bitarray'].frozenbitarray):
            raise TypeError('Expected a bitarray object')

        if self.model.ref and data.endian() != 'little':
            raise ValueError('A little endian bitarray object is expected for reflected CRCs')

        if not self.model.ref and data.endian() != 'big':
            raise ValueError('A big endian bitarray object is expected for non-reflected CRCs')

        if init is None:
            init = self.model.init

        cdef const unsigned char[:] view = data
        return crc_slice16(&self.model, init, &view[0], len(data))

    def combine(self, crc1, crc2, length):
        return crc_combine(&self.model, crc1, crc2, length * 8)

    def combine_bits(self, crc1, crc2, length):
        return crc_combine(&self.model, crc1, crc2, length)

    #byte-by-byte (for testing)
    def _calc_b(self, data):
        cdef const unsigned char[:] view = data
        return crc_bytewise(&self.model, self.model.init, &view[0], len(view) * 8)

def CRC(width=None, poly=None, init=None, refin=None, refout=None, xorout=None, check=None):
    names = ('width', 'poly', 'init', 'refin', 'refout', 'xorout')
    values = (width, poly, init, refin, refout, xorout)

    for name, value in zip(names, values):
        if value is None:
            raise ValueError(f'{name} value is not provided')

    if width > WORDBITS:
        raise ValueError(f'width is larger than {WORDBITS} bits')

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
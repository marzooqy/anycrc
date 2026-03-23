# Copyright (c) 2024-2026 Hussain Al Marzooq

# cython: freethreading_compatible = True

from libc.stdint cimport *
from functools import lru_cache
from bitarray import bitarray, frozenbitarray
from .models import models, aliases

cdef extern from '../../lib/crc-clmul/crc.h':
    ctypedef struct params_t:
        uint8_t width
        uint64_t poly
        bint refin, refout
        uint64_t init, xorout, k1, k2
        uint64_t table[256]
        uint64_t combine_table[64]

    cdef params_t crc_params(uint8_t width, uint64_t poly, uint64_t init, bint refin, bint refout, uint64_t xorout, uint64_t check, uint8_t *error)

    cdef uint64_t crc_table(params_t *params, uint64_t crc, const unsigned char *buf, uint64_t len)
    cdef uint64_t crc_calc(params_t *params, uint64_t crc, const unsigned char *buf, uint64_t len)
    cdef uint64_t crc_calc_bits(params_t *params, uint64_t crc, const unsigned char *buf, uint64_t len)

    cdef uint64_t crc_combine_constant(params_t *params, uint64_t len)
    cdef uint64_t crc_combine_constant_bits(params_t *params, uint64_t len)
    cdef uint64_t crc_combine_fixed(params_t *params, uint64_t crc, uint64_t crc2, uint64_t xp)

cdef class _CRC:
    cdef params_t params

    def __cinit__(self, width, poly, init, refin, refout, xorout, check=None):
        cdef uint8_t error
        self.params = crc_params(width, poly, init, refin, refout, xorout, check if check is not None else 0, &error)

        if error & 1:
            raise ValueError('CRC width should be between 1 and 64 bits')

        if check is not None and error & 0x20:
            raise ValueError('Invalid paramaters. check mismatch')

    def calc(self, data, init=None):
        if isinstance(data, bitarray) or isinstance(data, frozenbitarray):
            raise TypeError('Bitarray objects are not allowed, use calc_bits() instead')

        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')

        if init is None:
            init = self.params.init

        if len(data) == 0:
            return init

        cdef const unsigned char[:] view = data
        return crc_calc(&self.params, init, &view[0], len(view))

    def calc_bits(self, data, init=None):
        if not isinstance(data, bitarray) and not isinstance(data, frozenbitarray):
            raise TypeError('Expected a bitarray object')

        if self.params.refin and data.endian != 'little':
            raise ValueError('A little endian bitarray object is expected for reflected CRCs')

        if not self.params.refin and data.endian != 'big':
            raise ValueError('A big endian bitarray object is expected for non-reflected CRCs')

        if init is None:
            init = self.params.init

        if len(data) == 0:
            return init

        cdef const unsigned char[:] view = data
        return crc_calc_bits(&self.params, init, &view[0], len(data))

    @lru_cache
    def _combine_constant(self, length):
        return crc_combine_constant(&self.params, length)

    @lru_cache
    def _combine_constant_bits(self, length):
        return crc_combine_constant_bits(&self.params, length)

    def combine(self, crc1, crc2, length):
        return crc_combine_fixed(&self.params, crc1, crc2, self._combine_constant(length))

    def combine_bits(self, crc1, crc2, length):
        return crc_combine_fixed(&self.params, crc1, crc2, self._combine_constant_bits(length))

    #byte-by-byte (for testing)
    def _calc_b(self, data):
        cdef const unsigned char[:] view = data
        return crc_table(&self.params, self.params.init, &view[0], len(view))

@lru_cache
def CRC(width=None, poly=None, init=None, refin=None, refout=None, xorout=None, check=None):
    names = ('width', 'poly', 'init', 'refin', 'refout', 'xorout')
    values = (width, poly, init, refin, refout, xorout)

    for name, value in zip(names, values):
        if value is None:
            raise ValueError(f'{name} value is not provided')

    return _CRC(*values)

def Model(name):
    if name in models:
        return CRC(*models[name])
    elif name in aliases:
        return CRC(*models[aliases[name]])
    else:
        raise ValueError('CRC model not found')
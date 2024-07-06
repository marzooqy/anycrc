# Copyright (c) 2024 Hussain Al Marzooq

from libc.stdint cimport uintmax_t
from .models import *

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
        word_t res, res_hi
        word_t *table_byte
        word_t *table_slice16

    cdef model_t get_model(unsigned short width, word_t poly, word_t init, char refin, char refout, word_t xorout, word_t check, word_t res)
    cdef model_t get_model_dbl(unsigned short width, word_t poly_hi, word_t poly, word_t init_hi, word_t init, char refin, char refout,
                               word_t xorout_hi, word_t xorout, word_t check_hi, word_t check, word_t res_hi, word_t res)

    cdef void free_model(model_t *model)

cdef extern from '../../lib/crcany/crc.h':
    cdef int crc_table_bytewise(model_t *model)
    cdef word_t crc_bytewise(model_t *model, word_t crc, const void* dat, size_t len);

    cdef int crc_table_slice16(model_t *model)
    cdef word_t crc_slice16(model_t *model, word_t crc, const void* dat, size_t len)

cdef extern from '../../lib/crcany/crcdbl.h':
    cdef int crc_table_bytewise_dbl(model_t *model)
    cdef void crc_bytewise_dbl(model_t *model, word_t *crc_hi, word_t *crc_lo, const unsigned char *buf, size_t len)

cdef word_t MASK = -1
word_bits = WORDBITS #accessible from python

cdef class CRC:
    cdef model_t model
    cdef word_t reg, reg_hi

    def __cinit__(self, width=None, poly=None, init=None, refin=None, refout=None, xorout=None, check=0, residue=0):
        if width is None:
            raise ValueError('width value is not provided')

        elif width <= 0:
            raise ValueError('width should be larger than zero')

        if poly is None:
            raise ValueError('poly value is not provided')

        if init is None:
            raise ValueError('init value is not provided')

        if refin is None:
            raise ValueError('refin value is not provided')

        if refout is None:
            raise ValueError('refout value is not provided')

        if xorout is None:
            raise ValueError('xorout value is not provided')

        cdef int error_code

        if width <= WORDBITS:
            self.model = get_model(width, poly, init, refin, refout, xorout, check, residue)
            error_code = crc_table_bytewise(&self.model)

            if error_code == 1:
                raise MemoryError('Out of memory error')

            error_code = crc_table_slice16(&self.model)

            if error_code == 1:
                raise MemoryError('Out of memory error')

            self.reg = self.model.init

        elif width <= WORDBITS * 2:
            self.model = get_model_dbl(width, poly >> WORDBITS, poly & MASK, init >> WORDBITS, init & MASK, refin, refout, xorout >> WORDBITS, xorout & MASK,
            check >> WORDBITS, check & MASK, residue >> WORDBITS, residue & MASK)

            error_code = crc_table_bytewise_dbl(&self.model)

            if error_code == 1:
                raise MemoryError('Out of memory error')

            self.reg = self.model.init
            self.reg_hi = self.model.init_hi

        else:
            raise ValueError('CRC width is larger than what is allowed')

    def __dealloc__(self):
        free_model(&self.model);

    def get(self):
        if self.model.width <= WORDBITS:
            return self.reg
        else:
            crc = self.reg_hi
            crc <<= WORDBITS
            crc += self.reg
            return crc

    def set(self, crc):
        self.reg = crc & MASK
        self.reg_hi = crc >> WORDBITS

    def reset(self):
        self.reg = self.model.init
        self.reg_hi = self.model.init_hi

    def calc(self, data):
        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')

        cdef const unsigned char *data_p = data
        cdef word_t crc_lo = self.reg
        cdef word_t crc_hi = self.reg_hi

        if self.model.width <= WORDBITS:
            return crc_slice16(&self.model, self.reg, data_p, len(data))

        else:
            crc_bytewise_dbl(&self.model, &crc_hi, &crc_lo, data_p, len(data))
            crc = crc_hi
            crc <<= WORDBITS
            crc += crc_lo
            return crc

    def update(self, data):
        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')

        cdef const unsigned char *data_p = data

        if self.model.width <= WORDBITS:
            self.reg = crc_slice16(&self.model, self.reg, data_p, len(data))
            return self.reg

        else:
            crc_bytewise_dbl(&self.model, &self.reg_hi, &self.reg, data_p, len(data))
            crc = self.reg_hi
            crc <<= WORDBITS
            crc += self.reg
            return crc

    #byte-by-byte (for testing)
    def _calc_b(self, data):
        cdef const unsigned char* data_p = data
        cdef word_t crc_lo = self.reg
        cdef word_t crc_hi = self.reg_hi

        if self.model.width <= WORDBITS:
            return crc_bytewise(&self.model, self.reg, data_p, len(data))
        else:
            crc_bytewise_dbl(&self.model, &crc_hi, &crc_lo, data_p, len(data))
            crc = crc_hi
            crc <<= WORDBITS
            crc += crc_lo
            return crc

def Model(name):
    if name in models:
        return CRC(*models[name])
    elif name in aliases:
        return CRC(*models[aliases[name]])
    else:
        raise ValueError('CRC model not found')
# Copyright (c) 2024 Hussain Al Marzooq

from .models import *

cdef extern from '../../lib/crcany/model.h':
    ctypedef unsigned int word_t
    cdef const unsigned short WORDBITS

    ctypedef struct model_t:
        unsigned short width
        char ref, rev
        word_t poly, init, xorout
        word_t *table

    cdef model_t get_model(unsigned short width, word_t poly, word_t init, char refin, char refout, word_t xorout)
    cdef char init_model(model_t *model)
    cdef void free_model(model_t *model)

cdef extern from '../../lib/crcany/crc.h':
    cdef void crc_table_bytewise(model_t *model)
    cdef word_t crc_bytewise(model_t *model, word_t crc, const void *dat, size_t len);

    cdef void crc_table_slice16(model_t *model)
    cdef word_t crc_slice16(model_t *model, word_t crc, const void *dat, size_t len)

cdef class CRC:
    cdef model_t model
    cdef word_t register

    def __cinit__(self, width=None, poly=None, init=None, refin=None, refout=None, xorout=None, check=None):
        if width is None:
            raise ValueError('width value is not provided')

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

        if width > WORDBITS:
            raise ValueError(f'width is larger than {WORDBITS} bits')

        self.model = get_model(width, poly, init, refin, refout, xorout)
        cdef char error_code = init_model(&self.model)

        if error_code == 1:
            raise MemoryError('Out of memory error')

        crc_table_bytewise(&self.model)
        crc_table_slice16(&self.model)

        self.register = self.model.init

    def __dealloc__(self):
        free_model(&self.model);

    def get(self):
        return self.register

    def set(self, crc):
        self.register = crc

    def reset(self):
        self.register = self.model.init

    cpdef word_t calc(self, data):
        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')

        cdef const unsigned char[:] view = data
        return crc_slice16(&self.model, self.register, &view[0], len(view) * 8)

    cpdef word_t calc_bits(self, data):
        cdef const unsigned char[:] view = data
        cdef word_t length = len(data)

        if self.model.ref and length % 8 > 0:
            raise ValueError('bit lengths are not supported with reflected CRCs')

        return crc_slice16(&self.model, self.register, &view[0], length)

    def update(self, data):
        self.register = self.calc(data)
        return self.register

    def update_bits(self, data):
        self.register = self.calc_bits(data)
        return self.register

    #byte-by-byte (for testing)
    def _calc_b(self, data):
        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')

        cdef const unsigned char[:] view = data
        return crc_bytewise(&self.model, self.register, &view[0], len(view) * 8)

def Model(name):
    if name in models:
        return CRC(*models[name])
    elif name in aliases:
        return CRC(*models[aliases[name]])
    else:
        raise ValueError('CRC model not found')

# Copyright (c) 2024 Hussain Al Marzooq

from libc.stdint cimport uintmax_t
from .models import models, aliases
import sys

ctypedef uintmax_t word_t

cdef extern from '../../lib/crcany/model.h':
    cdef const unsigned short WORDCHARS
    cdef const unsigned short WORDBITS
    
    ctypedef struct model_t:
        unsigned short width
        short cycle
        short back
        char ref
        char rev
        char *name
        word_t poly, poly_hi
        word_t init, init_hi
        word_t xorout, xorout_hi
        word_t check, check_hi
        word_t res, res_hi
        word_t *table_comb
        word_t *table_byte
        word_t *table_word
        word_t *table_slice16
        
    cdef int read_model(model_t *model, char *str, int lenient)
    cdef void process_model(model_t *model)
    cdef void free_model(model_t *model)
    
cdef extern from '../../lib/crcany/crc.h':
    cdef int crc_table_bytewise(model_t *model)
    cdef word_t crc_bytewise(model_t *model, word_t crc, const void* dat, size_t len);
    
    cdef int crc_table_slice16(model_t *model, unsigned little, unsigned word_bits)
    cdef word_t crc_slice16(model_t *model, word_t crc, const void* dat, size_t len)
    
    cdef word_t crc_parallel(model_t *model, word_t crc, const void *dat, size_t len, int *error)
    
    cdef int crc_table_combine(model_t *model)
    cdef word_t crc_combine(model_t *model, word_t crc1, word_t crc2, uintmax_t len2);
    
cdef extern from '../../lib/crcany/crcdbl.h':
    cdef int crc_table_bytewise_dbl(model_t *model)
    cdef void crc_bytewise_dbl(model_t *model, word_t *crc_hi, word_t *crc_lo, const unsigned char *buf, size_t len)
    
cdef bint parallel = True
cdef word_t MASK = -1
word_bits = WORDBITS

cdef class CRC:
    cdef model_t model
    cdef word_t reg, reg_hi
    
    def __cinit__(self, width, poly, init, ref_in, ref_out, xor_out, check=0, residue=0):
        cdef unsigned endian = 1 if sys.byteorder == 'little' else 0
        refin = 'true' if ref_in else 'false'
        refout = 'true' if ref_out else 'false'
        
        if width > WORDBITS * 2:
            raise ValueError('CRC width is larger than what is allowed')
            
        string = f'width={width} poly={poly} init={init} refin={refin} refout={refout} xorout={xor_out} check={check} residue={residue} name=""'.encode('utf-8')
        
        cdef int error_code = read_model(&self.model, string, 0)
        
        if error_code != 0:
            raise ValueError('An error occurred while retrieving the model, check the arguments passed to the CRC class')
            
        process_model(&self.model)
        
        if self.model.width <= WORDBITS:
            error_code = crc_table_bytewise(&self.model)
            
            if error_code == 1:
                raise MemoryError('Out of memory error')
                
            error_code = crc_table_slice16(&self.model, endian, WORDBITS)
            
            if error_code == 1:
                raise MemoryError('Out of memory error')
                
            error_code = crc_table_combine(&self.model)
            
            if error_code == 1:
                raise MemoryError('Out of memory error')
                
            self.reg = self.model.init
            
        else:
            error_code = crc_table_bytewise_dbl(&self.model)
            
            if error_code == 1:
                raise MemoryError('Out of memory error')
                
            self.reg = self.model.init
            self.reg_hi = self.model.init_hi
            
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
        
    cdef word_t _calc(self, const unsigned char *data, word_t length):
        cdef word_t crc
        cdef int error = 0
        
        if parallel and length >= 200000:
            crc = crc_parallel(&self.model, self.reg, data, length, &error)
            
            if error == 1:
                raise MemoryError('Out of memory error')
                
            return crc
            
        else:
            return crc_slice16(&self.model, self.reg, data, length)
            
    def calc(self, data):
        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')
            
        cdef word_t crc_lo = self.reg
        cdef word_t crc_hi = self.reg_hi
        
        if self.model.width <= WORDBITS:
            return self._calc(data, len(data))
            
        else:
            crc_bytewise_dbl(&self.model, &crc_hi, &crc_lo, data, len(data))
            crc = crc_hi
            crc <<= WORDBITS
            crc += crc_lo
            return crc
            
    def update(self, data):
        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')
            
        if self.model.width <= WORDBITS:
            self.reg = self._calc(data, len(data))
            return self.reg
            
        else:
            crc_bytewise_dbl(&self.model, &self.reg_hi, &self.reg, data, len(data))
            crc = self.reg_hi
            crc <<= WORDBITS
            crc += self.reg
            return crc
            
    #parallel
    def _calc_p(self, data):
        cdef const unsigned char* data_p = data
        cdef int error = 0
        cdef crc = crc_parallel(&self.model, self.reg, data_p, len(data), &error)
        
        if error == 1:
            raise MemoryError('Out of memory error')
            
        return crc
        
    #slice-by-16
    def _calc_16(self, data):
        cdef const unsigned char* data_p = data
        return crc_slice16(&self.model, self.reg, data_p, len(data))
        
    #byte-by-byte
    def _calc_b(self, data):
        cdef const unsigned char* data_p = data
        return crc_bytewise(&self.model, self.reg, data_p, len(data))
        
def set_parallel(is_parallel):
    global parallel
    parallel = is_parallel
    
def Model(name):
    if name in models:
        return CRC(*models[name])
    elif name in aliases:
        return CRC(*models[aliases[name]])
    else:
        raise ValueError('CRC model not found')
#  Copyright (c) 2024 Hussain Al Marzooq

from libc.stdint cimport uintmax_t
from .models import models
import sys

ctypedef uintmax_t word_t

cdef extern from '../../lib/crcany/model.h':
    cdef const unsigned char WORDCHARS
    
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
        word_t table_comb[67]
        word_t table_byte[256]
        word_t table_word[WORDCHARS][256]
        
    cdef int read_model(model_t *model, char *str, int lenient)
    cdef void process_model(model_t *model)
    
cdef extern from '../../lib/crcany/crc.h':
    cdef void crc_table_bytewise(model_t *model)
    cdef void crc_table_wordwise(model_t *model, unsigned little, unsigned bits)
    cdef word_t crc_wordwise(model_t *model, word_t crc, const void* dat, size_t len)

cdef class CRC:
    cdef model_t model
    cdef word_t register
    
    def __init__(self, unsigned short width, word_t poly, word_t init, char ref_in, char ref_out, word_t xor_out, word_t check=0, word_t residue=0, name=''):
        refin = 'true' if ref_in else 'false'
        refout = 'true' if ref_out else 'false'
        
        string = f'width={width} poly={poly} init={init} refin={refin} refout={refout} xorout={xor_out} check={check} residue={residue} name="{name}"'.encode('utf-8')
        cdef int error_code = read_model(&self.model, string, 0)
        
        if error_code != 0:
            raise ValueError('An error occurred while retrieving the model, check the arguments passed to the CRC class')
            
        process_model(&self.model)
        crc_table_bytewise(&self.model)
        
        cdef unsigned endian = 1 if sys.byteorder == 'little' else 0
        cdef unsigned word_size = 64 if sys.maxsize > 2 ** 32 else 32
        
        crc_table_wordwise(&self.model, endian , word_size)
        self.register = self.model.init
        
    def calc(self, data):
        if isinstance(data, str):
            data = (<unicode> data).encode('utf-8')
            
        cdef unsigned char* dat = data
        cdef crc = crc_wordwise(&self.model, self.register, dat, len(data))
        self.register = crc
        return crc
        
    def reset(self):
        self.register = self.model.init
        
def Model(name):
    return CRC(*models[name])
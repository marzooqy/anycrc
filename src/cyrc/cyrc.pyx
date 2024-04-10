#  Copyright (c) 2024  Hussain Al Marzooq

# The original pycrc library has the following license:

#  pycrc -- parameterisable CRC calculation utility and C source code generator
#
#  Copyright (c) 2006-2017  Thomas Pircher  <tehpeh-web@tty1.net>
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to
#  deal in the Software without restriction, including without limitation the
#  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
#  sell copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
#  IN THE SOFTWARE.

ctypedef unsigned long long uint

cdef class Model:
    """
    A base class for CRC routines.
    """
    
    cdef uint width, poly, reflect_in, xor_in, reflect_out, xor_out
    cdef uint msb_mask, mask, crc_shift
    cdef uint[256] tbl
    
    def __init__(self, uint width, uint poly, uint reflect_in, uint xor_in, uint reflect_out, uint xor_out):
        """
        Create a CRC object, using the Rocksoft model.
        """
        self.width = width
        self.poly = poly
        self.reflect_in = reflect_in
        self.xor_in = xor_in
        self.reflect_out = reflect_out
        self.xor_out = xor_out
        
        self.msb_mask = (<uint> 1) << (self.width - 1)
        self.mask = ((self.msb_mask - 1) << 1) | 1
        
        if self.width < 8:
            self.crc_shift = 8 - self.width
        else:
            self.crc_shift = 0
            
        self.gen_table(self.tbl)
        
    cdef uint reflect(self, uint data, uint width):
        """
        Reflect a data word, i.e. reverts the bit order.
        """
        cdef uint res = data & 1
        for dummy_i in range(width - 1):
            data >>= 1
            res = (res << 1) | (data & 1)
            
        return res
        
    cdef gen_table(self, uint *tbl):
        """
        This function generates the CRC table used for the table_driven CRC algorithm.
        """
        cdef uint reg
        
        for i in range(256):
            reg = i
            if self.reflect_in:
                reg = self.reflect(reg, 8)
                
            reg = reg << (self.width - 8 + self.crc_shift)
            
            for dummy_j in range(8):
                if reg & (self.msb_mask << self.crc_shift) != 0:
                    reg = (reg << 1) ^ (self.poly << self.crc_shift)
                else:
                    reg = (reg << 1)
                
            if self.reflect_in:
                reg = self.reflect(reg >> self.crc_shift, self.width) << self.crc_shift
                
            tbl[i] = (reg >> self.crc_shift) & self.mask

    def init(self):
        return self.xor_in

    def update(self, uint crc, in_data):
        """
        Update the crc value with new data.
        """
        cdef const unsigned char[:] data
        
        # If the input data is a string, convert to bytes.
        if isinstance(in_data, str):
            data = (<unicode> in_data).encode('utf-8')
        else:
            data = in_data
        
        cdef uint reg = crc
        cdef uint tblidx
        
        if not self.reflect_in:
            reg = reg << self.crc_shift
            
            for octet in data:
                tblidx = ((reg >> (self.width - 8 + self.crc_shift)) ^ octet) & 0xff
                reg = ((reg << (8 - self.crc_shift)) ^
                       (self.tbl[tblidx] << self.crc_shift)) & (self.mask << self.crc_shift)
                       
            reg = reg >> self.crc_shift
            
        else:
            reg = self.reflect(reg, self.width)
            
            for octet in data:
                tblidx = (reg ^ octet) & 0xff
                reg = ((reg >> 8) ^ self.tbl[tblidx]) & self.mask
                
            reg = self.reflect(reg, self.width) & self.mask
            
        return reg

    def finalize(self, uint crc):
        """
        Calculate the final crc value.
        """
        
        if self.reflect_out:
            crc = self.reflect(crc, self.width)
            
        return crc ^ self.xor_out
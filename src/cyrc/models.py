#  Copyright (c) 2024  Hussain Al Marzooq

# The original pycrc library had the following license:

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

from cyrc import Model

crc5 = Model(5, 0x05, True, 0x1f, True, 0x1f)
crc5_check = 0x19

crc8 = Model(8, 0x07, False, 0x0, False, 0x0)
crc8_check = 0xf4

dallas_1wire = Model(8, 0x31, True, 0x0, True, 0x0)
dallas_1wire_check = 0xa1

crc12_3gpp = Model(12, 0x80f, False, 0x0, True, 0x0)
crc12_3gpp_check = 0xdaf

crc15 = Model(15, 0x4599, False, 0x0, False, 0x0)
crc15_check = 0x59e

crc16 = Model(16, 0x8005, True, 0x0, True, 0x0)
crc16_check = 0xbb3d

crc16_usb = Model(16, 0x8005, True, 0xffff, True, 0xffff)
crc16_usb_check = 0xb4c8

crc16_modbus = Model(16, 0x8005, True, 0xffff, True, 0x0)
crc16_modbus_check = 0x4b37

crc16_genibus = Model(16, 0x1021, False, 0xffff, False, 0xffff)
crc16_genibus_check = 0xd64e

crc16_ccitt = Model(16, 0x1021, False, 0x1d0f, False, 0x0)
crc16_ccitt_check = 0xe5cc

r_crc16 = Model(16, 0x0589, False, 0x0, False, 0x0001)
r_crc16_check = 0x007e

kermit = Model(16, 0x1021, True, 0x0, True, 0x0)
kermit_check = 0x2189

x25 = Model(16, 0x1021, True, 0xffff, True, 0xffff)
x25_check = 0x906e

xmodem = Model(16, 0x1021, False, 0x0, False, 0x0)
xmodem_check = 0x31c3

zmodem = Model(16, 0x1021, False, 0x0, False, 0x0)
zmodem_check = 0x31c3

crc24 = Model(24, 0x864cfb, False, 0xb704ce, False, 0x0)
crc24_check = 0x21cf02

crc32 = Model(32, 0x4c11db7, True, 0xffffffff, True, 0xffffffff)
crc32_check = 0xcbf43926

crc32c = Model(32, 0x1edc6f41, True, 0xffffffff, True, 0xffffffff)
crc32c_check = 0xe3069283

crc32_mpeg = Model(32, 0x4c11db7, False, 0xffffffff, False, 0x0)
crc32_mpeg_check = 0x0376e6e7

crc32_bzip2 = Model(32, 0x04c11db7, False, 0xffffffff, False, 0xffffffff)
crc32_bzip2_check = 0xfc891918

posix = Model(32, 0x4c11db7, False, 0x0, False, 0xffffffff)
posix_check = 0x765e7680

jam = Model(32, 0x4c11db7, True, 0xffffffff, True, 0x0)
jam_check = 0x340bc6d9

xfer = Model(32, 0x000000af, False, 0x0, False, 0x0)
xfer_check = 0xbd0be338

crc64 = Model(64, 0x000000000000001b, True, 0x0, True, 0x0,)
crc64_check = 0x46a5a9388a5beffe

crc64_jones = Model(64, 0xad93d23594c935a9, True, 0xffffffffffffffff, True, 0x0)
crc64_jones_check = 0xcaa717168609f281

crc64_xz = Model(64, 0x42f0e1eba9ea3693, True, 0xffffffffffffffff, True, 0xffffffffffffffff)
crc64_xz_check = 0x995dc9bbdf1939fa
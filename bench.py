import binascii
import crcmod
import crcmod.predefined
import cyrc
from cyrc.models import crc32
import fastcrc
import time

test_data = b"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."

n = 10 ** 7

print()

#cyrc
t = time.perf_counter()

for i in range(n):
    val =crc32.init()
    val = crc32.update(val, test_data)
    crc32.finalize(val)

cyrc_time_elapsed = time.perf_counter() - t
avg_time = cyrc_time_elapsed / n

print('cyrc')
print('Time Elapsed: {:.3f}s'.format(cyrc_time_elapsed))
print('Average: {:.3f} us/run'.format(avg_time * 10 ** 6))
print('Relative: {:.3f}'.format(1))
print()

#binascii
t = time.perf_counter()

for i in range(n):
    binascii.crc32(test_data)

time_elapsed = time.perf_counter() - t
avg_time = time_elapsed / n

print('binascii')
print('Time Elapsed: {:.3f}s'.format(time_elapsed))
print('Average: {:.3f} us/run'.format(avg_time * 10 ** 6))
print('Relative: {:.3f}'.format(time_elapsed / cyrc_time_elapsed))
print()

#fastcrc
t = time.perf_counter()

for i in range(n):
    fastcrc.crc32.iso_hdlc(test_data)

time_elapsed = time.perf_counter() - t
avg_time = time_elapsed / n

print('fastcrc')
print('Time Elapsed: {:.3f}s'.format(time_elapsed))
print('Average: {:.3f} us/run'.format(avg_time * 10 ** 6))
print('Relative: {:.3f}'.format(time_elapsed / cyrc_time_elapsed))
print()

#crcmod
t = time.perf_counter()

calc = crcmod.predefined.mkPredefinedCrcFun('crc-32')
for i in range(n):
    calc(test_data)

time_elapsed = time.perf_counter() - t
avg_time = time_elapsed / n

print('crcmod-plus')
print('Time Elapsed: {:.3f}s'.format(time_elapsed))
print('Average: {:.3f} us/run'.format(avg_time * 10 ** 6))
print('Relative: {:.3f}'.format(time_elapsed / cyrc_time_elapsed))
print()
import anycrc
import crcmod.predefined
import fastcrc

import random
import time
import zlib

test_data = bytes(random.randint(0, 255) for i in range(10000))

def benchmark(data, n):
    modules = ['anycrc', 'zlib', 'fastcrc', 'crcmod-plus']
    duration = [0] * len(modules)
    speed = [0] * len(modules)
    relative = [0] * len(modules)

    print()

    test_n = 0

    #anycrc
    t = time.perf_counter()

    model = anycrc.Model('CRC32')

    for i in range(n):
        model.calc(data)

    anycrc_time_elapsed = time.perf_counter() - t

    duration[test_n] = anycrc_time_elapsed
    speed[test_n] = len(data) * n / (1024 ** 3) / anycrc_time_elapsed
    relative[test_n] = 1

    print(modules[test_n])
    print('Time Elapsed: {:.2f}s'.format(duration[test_n]))
    print('Speed: {:.2f} GiB/s'.format(speed[test_n]))
    print('Relative: {:.2f}'.format(relative[test_n]))
    print()

    test_n += 1

    #zlib
    t = time.perf_counter()

    for i in range(n):
        zlib.crc32(data)

    time_elapsed = time.perf_counter() - t

    duration[test_n] = time_elapsed
    speed[test_n] = len(data) * n / (1024 ** 3) / time_elapsed
    relative[test_n] = time_elapsed / anycrc_time_elapsed

    print(modules[test_n])
    print('Time Elapsed: {:.2f}s'.format(duration[test_n]))
    print('Speed: {:.2f} GiB/s'.format(speed[test_n]))
    print('Relative: {:.2f}'.format(relative[test_n]))
    print()

    test_n += 1

    #fastcrc
    t = time.perf_counter()

    for i in range(n):
        fastcrc.crc32.iso_hdlc(data)

    time_elapsed = time.perf_counter() - t

    duration[test_n] = time_elapsed
    speed[test_n] = len(data) * n / (1024 ** 3) / time_elapsed
    relative[test_n] = time_elapsed / anycrc_time_elapsed

    print(modules[test_n])
    print('Time Elapsed: {:.2f}s'.format(duration[test_n]))
    print('Speed: {:.2f} GiB/s'.format(speed[test_n]))
    print('Relative: {:.2f}'.format(relative[test_n]))
    print()

    test_n += 1

    #crcmod
    t = time.perf_counter()

    calc = crcmod.predefined.mkPredefinedCrcFun('crc-32')
    for i in range(n):
        calc(data)

    time_elapsed = time.perf_counter() - t

    duration[test_n] = time_elapsed
    speed[test_n] = len(data) * n / (1024 ** 3) / time_elapsed
    relative[test_n] = time_elapsed / anycrc_time_elapsed

    print(modules[test_n])
    print('Time Elapsed: {:.2f}s'.format(duration[test_n]))
    print('Speed: {:.2f} GiB/s'.format(speed[test_n]))
    print('Relative: {:.2f}'.format(relative[test_n]))
    print()

    print("| Module | Time Elapsed (s) | Speed (GiB/s) | Relative |")
    print("|---|:-:|:-:|:-:|")

    for i in range(len(modules)):
        print('| {} | {:.2f} | {:.2f} | {:.2f} |'.format(modules[i], duration[i], speed[i], relative[i]))

    print()

benchmark(test_data, 100000)

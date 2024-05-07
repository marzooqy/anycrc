import anycrc
import crcmod
import crcmod.predefined
import fastcrc

import time
import sys
import zlib

test_data = b"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."

def benchmark(data, n):
    modules = ['anycrc (parallel)', 'anycrc (serial)', 'zlib', 'fastcrc', 'crcmod-plus']
    duration = [0] * len(modules)
    speed = [0] * len(modules)
    relative = [0] * len(modules)
    
    print()
    
    test_n = 0
    
    #anycrc parallel
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
    
    #anycrc serial
    t = time.perf_counter()
    
    anycrc.set_parallel(False)
    model = anycrc.Model('CRC32')

    for i in range(n):
        model.calc(data)
        
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
    
if len(sys.argv) == 1:
    print('Enter the number of the test as a parameter\n')
    
elif sys.argv[1] == '1':
    benchmark(test_data, 10 ** 6)
    
elif sys.argv[1] == '2':
    benchmark(test_data * 10 ** 6, 1)
    
#find the point at which the parallel version becomes faster than the serial version
elif sys.argv[1] == '3':
    model = anycrc.Model('CRC32')
    n = 100000
    
    for i in range(4):
        t = time.perf_counter()
        data = test_data[:200] * 10 ** i
        
        print('{} Bytes:'.format(len(data)))
        
        for i in range(n):
            model._calc_16(data)

        s_time_elapsed = time.perf_counter() - t
        
        print('Serial: {:.2f}s'.format(s_time_elapsed))
        
        t = time.perf_counter()
        
        for i in range(n):
            model._calc_p(data)

        p_time_elapsed = time.perf_counter() - t
        
        print('Parallel: {:.2f}s\n'.format(p_time_elapsed))

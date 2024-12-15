import random
import time

from pycrc import algorithms
import anycrc
import crcmod.predefined
import fastcrc
import crcct #renamed from crc
import crcengine
import crc

#requires a C compiler
try:
    import libscrc
except ModuleNotFoundError:
    pass

N = 10000
LENGTH = 10000

class Benchmark:
    def __init__(self, module, relative_to=None):
        self.module = module
        self.duration = None
        self.relative_to = relative_to

    def __repr__(self):
        return f'{self.module}\nTime Elapsed: {self.duration:.2f}s\nSpeed: {self.get_speed():.2f} MB/s\nRelative: {self.get_relative():.2f}'

    def get_speed(self):
        return len(data) * N / (10 ** 6) / self.duration

    def get_relative(self):
        if self.relative_to is None:
            return 1
        else:
            return self.duration / self.relative_to

benchmarks = []

data = bytes(random.randint(0, 255) for i in range(LENGTH))

print()

#anycrc
benchmark = Benchmark('anycrc')
t = time.perf_counter()

model = anycrc.Model('CRC32')

for i in range(N):
    model.calc(data)

anycrc_duration = time.perf_counter() - t
benchmark.duration = anycrc_duration
benchmarks.append(benchmark)

print(benchmark)
print()

#fastcrc
benchmark = Benchmark('fastcrc', relative_to=anycrc_duration)
t = time.perf_counter()

for i in range(N):
    fastcrc.crc32.iso_hdlc(data)

benchmark.duration = time.perf_counter() - t
benchmarks.append(benchmark)

print(benchmark)
print()

#crcmod
benchmark = Benchmark('crcmod-plus', relative_to=anycrc_duration)
t = time.perf_counter()

calc = crcmod.predefined.mkPredefinedCrcFun('crc-32')
for i in range(N):
    calc(data)

benchmark.duration = time.perf_counter() - t
benchmarks.append(benchmark)

print(benchmark)
print()

#crc-ct
benchmark = Benchmark('crc-ct', relative_to=anycrc_duration)
t = time.perf_counter()

crc_model = crcct.predefined_model_by_name(b'CRC-32')
for i in range(N):
    crc_result = crcct.init(crc_model)
    crc_result = crcct.update(crc_model, data, len(data), crc_result)
    crcct.final(crc_model, crc_result)

benchmark.duration = time.perf_counter() - t
benchmarks.append(benchmark)

print(benchmark)
print()

#libscrc
try:
    benchmark = Benchmark('libscrc', relative_to=anycrc_duration)
    t = time.perf_counter()

    for i in range(N):
        libscrc.crc32(data)

    benchmark.duration = time.perf_counter() - t
    benchmarks.append(benchmark)

    print(benchmark)
    print()

except NameError:
    pass

#crcengine
benchmark = Benchmark('crcengine', relative_to=anycrc_duration)
t = time.perf_counter()

crc_algorithm = crcengine.new('crc32')
for i in range(N):
    crc_algorithm(data)

benchmark.duration = time.perf_counter() - t
benchmarks.append(benchmark)

print(benchmark)
print()

#pycrc
benchmark = Benchmark('pycrc', relative_to=anycrc_duration)
t = time.perf_counter()

CRC = algorithms.Crc(width=32, poly=0x4c11db7, reflect_in=True, xor_in=0xffffffff, reflect_out=True, xor_out=0xffffffff)
for i in range(N):
    CRC.table_driven(data)

benchmark.duration = time.perf_counter() - t
benchmarks.append(benchmark)

print(benchmark)
print()

#crc
benchmark = Benchmark('crc', relative_to=anycrc_duration)
t = time.perf_counter()

calculator = crc.Calculator(crc.Crc32.CRC32, optimized=True)
for i in range(N):
    calculator.checksum(data)

benchmark.duration = time.perf_counter() - t
benchmarks.append(benchmark)

print(benchmark)
print()

benchmarks.sort(key=lambda banchmark: benchmark.duration)

file_name = 'benchmark_results.txt'

with open(file_name, 'w') as file:
    file.write('| Module | Time (s) | Speed (MB/s) | Relative |\n')
    file.write('|---|:-:|:-:|:-:|\n')

    for benchmark in benchmarks:
        file.write(f'| {benchmark.module} | {benchmark.duration:.2f} | {benchmark.get_speed():.2f} | x{benchmark.get_relative():.2f} |\n')

print(f'Results saved to "{file_name}"')
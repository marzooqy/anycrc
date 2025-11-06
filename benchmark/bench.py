try:
    import time

    from pycrc import algorithms
    import anycrc
    import crcmod.predefined
    import fastcrc
    import libscrc
    import crcengine
    import crccheck
    import crc

    N_C_EXT = 10000
    N_PYTHON = 100
    LENGTH = 65536

    class Benchmark:
        def __init__(self, module, n, relative_to=None):
            self.module = module
            self.n = n
            self.average = None
            self.relative_to = relative_to
            self.start = time.perf_counter()

        def __repr__(self):
            return f'{self.module}\nTime: {self.average * self.n:.2f}s\nSpeed: {self.get_speed():.2f} MiB/s\nRelative: {self.get_relative():.2f}'

        def stop(self):
            t = time.perf_counter()
            self.average = (t - self.start) / self.n
            self.start = t

        def get_speed(self):
            return LENGTH / self.average / (1024 ** 2)

        def get_relative(self):
            if self.relative_to is None:
                return 1
            else:
                return self.average / self.relative_to.average

    benchmarks = []

    data = bytes(i & 0xff for i in range(LENGTH))

    print(f'\nN = {N_C_EXT}\n')

    #anycrc
    anycrc_benchmark = Benchmark('anycrc', N_C_EXT)

    model = anycrc.Model('CRC32')
    for i in range(anycrc_benchmark.n):
        model.calc(data)

    anycrc_benchmark.stop()
    benchmarks.append(anycrc_benchmark)

    print(anycrc_benchmark)
    print()

    #fastcrc
    benchmark = Benchmark('fastcrc', N_C_EXT, relative_to=anycrc_benchmark)

    for i in range(benchmark.n):
        fastcrc.crc32.iso_hdlc(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #crcmod
    benchmark = Benchmark('crcmod-plus', N_C_EXT, relative_to=anycrc_benchmark)

    calc = crcmod.predefined.mkPredefinedCrcFun('crc-32')
    for i in range(benchmark.n):
        calc(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #libscrc
    benchmark = Benchmark('libscrc', N_C_EXT, relative_to=anycrc_benchmark)

    for i in range(benchmark.n):
        libscrc.crc32(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #use smaller N for slower pure python libraries
    print(f'N = {N_PYTHON}\n')

    #crcengine
    benchmark = Benchmark('crcengine', N_PYTHON, relative_to=anycrc_benchmark)

    crc_algorithm = crcengine.new('crc32')
    for i in range(benchmark.n):
        crc_algorithm(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #pycrc
    benchmark = Benchmark('pycrc', N_PYTHON, relative_to=anycrc_benchmark)

    CRC = algorithms.Crc(width=32, poly=0x4c11db7, reflect_in=True, xor_in=0xffffffff, reflect_out=True, xor_out=0xffffffff)
    for i in range(benchmark.n):
        CRC.table_driven(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #crccheck
    benchmark = Benchmark('crccheck', N_PYTHON, relative_to=anycrc_benchmark)

    for i in range(benchmark.n):
        crcinst = crccheck.crc.Crc32()
        crcinst.process(data)
        crcinst.final()

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #crc
    benchmark = Benchmark('crc', N_PYTHON, relative_to=anycrc_benchmark)

    calculator = crc.Calculator(crc.Crc32.CRC32, optimized=True)
    for i in range(benchmark.n):
        calculator.checksum(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    benchmarks.sort(key=lambda benchmark: benchmark.get_speed(), reverse=True)

    file_name = 'benchmark_results.txt'

    with open(file_name, 'w') as file:
        file.write('| Module | Speed (MiB/s) | Relative |\n')
        file.write('|---|:-:|:-:|\n')

        for benchmark in benchmarks:
            file.write(f'| {benchmark.module} | {benchmark.get_speed():.2f} | x{benchmark.get_relative():.2f} |\n')

    print(f'Results saved to "{file_name}"')

except KeyboardInterrupt:
    pass
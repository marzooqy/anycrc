try:
    import time
    import zlib

    from pycrc import algorithms
    import pycrc32
    import crc32c
    import google_crc32c
    import anycrc
    import crcmod.predefined
    import fastcrc
    import crc_ct #renamed from crc
    import crcengine
    import crccheck
    import crc

    #requires a C compiler
    try:
        import libscrc
    except ModuleNotFoundError:
        pass

    N = 1000
    LENGTH = 10000

    class Benchmark:
        def __init__(self, module, note='', relative_to=None):
            self.module = module
            self.note = note
            self.duration = None
            self.relative_to = relative_to
            self.start = time.perf_counter()
            self.ran_once = False

        def __repr__(self):
            if self.ran_once:
                return f'{self.module}\nTime: {self.duration:.2f}s\nSpeed: {self.get_speed():.2f} MB/s\nRelative: {self.get_relative():.2f}'
            else:
                raise Exception("Benchmark was not completed")

        def stop(self):
            t = time.perf_counter()
            self.duration = t - self.start
            self.ran_once = True
            self.start = t

        def get_speed(self):
            return LENGTH * N / self.duration / (10 ** 6)

        def get_relative(self):
            if self.relative_to is None:
                return 1
            else:
                return self.duration / self.relative_to.duration

    benchmarks = []

    data = bytes(i & 0xff for i in range(LENGTH))

    print()

    #anycrc
    anycrc_benchmark = Benchmark('anycrc')

    model = anycrc.Model('CRC32')
    for i in range(N):
        model.calc(data)

    anycrc_benchmark.stop()
    benchmarks.append(anycrc_benchmark)

    print(anycrc_benchmark)
    print()

    #pycrc32
    benchmark = Benchmark('pycrc32', 'CRC32 only', relative_to=anycrc_benchmark)

    for i in range(N):
        pycrc32.crc32(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #crc32c
    benchmark = Benchmark('crc32c', 'CRC32C only', relative_to=anycrc_benchmark)

    for i in range(N):
        crc32c.crc32c(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #google-crc32c
    benchmark = Benchmark('google-crc32c', 'CRC32C only', relative_to=anycrc_benchmark)

    for i in range(N):
        google_crc32c.Checksum(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #zlib
    benchmark = Benchmark('zlib', 'CRC32 only', relative_to=anycrc_benchmark)

    for i in range(N):
        zlib.crc32(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #fastcrc
    benchmark = Benchmark('fastcrc', relative_to=anycrc_benchmark)

    for i in range(N):
        fastcrc.crc32.iso_hdlc(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #crcmod
    benchmark = Benchmark('crcmod-plus', relative_to=anycrc_benchmark)

    calc = crcmod.predefined.mkPredefinedCrcFun('crc-32')
    for i in range(N):
        calc(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #crc-ct
    benchmark = Benchmark('crc-ct', relative_to=anycrc_benchmark)

    crc_model = crc_ct.predefined_model_by_name(b'CRC-32')
    for i in range(N):
        crc_result = crc_ct.init(crc_model)
        crc_result = crc_ct.update(crc_model, data, len(data), crc_result)
        crc_ct.final(crc_model, crc_result)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #libscrc
    try:
        benchmark = Benchmark('libscrc', relative_to=anycrc_benchmark)

        for i in range(N):
            libscrc.crc32(data)

        benchmark.stop()
        benchmarks.append(benchmark)

        print(benchmark)
        print()

    except NameError:
        pass

    #crcengine
    benchmark = Benchmark('crcengine', relative_to=anycrc_benchmark)

    crc_algorithm = crcengine.new('crc32')
    for i in range(N):
        crc_algorithm(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #pycrc
    benchmark = Benchmark('pycrc', relative_to=anycrc_benchmark)

    CRC = algorithms.Crc(width=32, poly=0x4c11db7, reflect_in=True, xor_in=0xffffffff, reflect_out=True, xor_out=0xffffffff)
    for i in range(N):
        CRC.table_driven(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #crccheck
    benchmark = Benchmark('crccheck', relative_to=anycrc_benchmark)

    for i in range(N):
        crcinst = crccheck.crc.Crc32()
        crcinst.process(data)
        crcinst.final()

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    #crc
    benchmark = Benchmark('crc', relative_to=anycrc_benchmark)

    calculator = crc.Calculator(crc.Crc32.CRC32, optimized=True)
    for i in range(N):
        calculator.checksum(data)

    benchmark.stop()
    benchmarks.append(benchmark)

    print(benchmark)
    print()

    benchmarks.sort(key=lambda benchmark: benchmark.duration)

    file_name = 'benchmark_results.txt'

    with open(file_name, 'w') as file:
        file.write('| Module | Speed (MB/s) | Relative | Notes |\n')
        file.write('|---|:-:|:-:|:-:|\n')

        for benchmark in benchmarks:
            file.write(f'| {benchmark.module} | {benchmark.get_speed():.2f} | x{benchmark.get_relative():.2f} | {benchmark.note} |\n')

    print(f'Results saved to "{file_name}"')

except KeyboardInterrupt:
    pass
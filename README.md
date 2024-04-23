This is a Cython module with bindings to the [crcany](https://github.com/madler/crcany) library. It supports calculating CRC hashes of arbitary sizes as well as updating a crc hash over time. It takes advantage of crcany's ability to efficiently combine multiple CRCs to parallelize the CRC's calculation.

## Installation

`pip install anycrc`

## Usage

Use an existing model:

```python
>>> import anycrc
>>> crc32 = anycrc.Model('CRC32-ISO-HDLC')
>>> crc32.calc(b'Hello World!')
472456355
```

Read the data in chunks:

```python
>>> crc32.reset()
>>> crc32.calc(b'Hello ')
3928882368
>>> crc32.calc(b'World!')
472456355
```

Specify the starting CRC value:

```python
>>> crc32.set(3928882368)
>>> crc32.calc('World!')
472456355
```

Specify your own CRC parameters:

```python
>>> # width, poly, init, refin, refout, xorout
>>> my_crc = anycrc.CRC(10, 0b0101010101, 0x3ff, True, False, 0)
>>> my_crc.calc('Hello World!')
35
```

This module will sometimes compute the CRC in parallel using multiple processors. To disable parallelism, use:

```python
>>> anycrc.set_parallel(False)
```

The CRC's width cannot exceed your system's maximum integer width.

For a list of pre-built models, check [models.py](https://github.com/marzooqy/anycrc/blob/main/src/anycrc/models.py). To get a list of the models at any time, use the following command:

`python -m anycrc models`

## Benchmarks

Calculating the CRC32 for lorem ipsum 10 million times:

| Module | Time Elapsed (s) | Speed (MiB/s) | Relative |
|---|:-:|:-:|:-:|
| anycrc | 2.397 | 1770.21 | 1.000 |
| zlib | 2.791 | 1520.45 | 1.164 |
| fastcrc | 7.782 | 545.37 | 3.246 |
| crcmod-plus | 8.891 | 477.35 | 3.708 |

Calculating the CRC32 for the text of lorem ipsum repeated 1 million times in a single pass:

| Module | Time Elapsed (s) | Speed (MiB/s) | Relative |
|---|:-:|:-:|:-:|
| anycrc (parallel) | 0.018 | 24231.05 | 1.000 |
| anycrc (serial) | 0.202 | 2100.68 | 11.535 |
| zlib | 0.215 | 1977.93 | 12.251 |
| fastcrc | 0.670 | 633.75 | 38.234 |
| crcmod-plus | 0.668 | 635.19 | 38.148 |

Tested on a 12th generation Intel i7 processor. Parallel performance will depend on your system.

#### Notes

\- Parallelism is disabled when the length of the input data is under 20k, as the serial method is faster in that case.

\- The macOS build only supports the serial version.

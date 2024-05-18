This is a Cython module with bindings to the [crcany](https://github.com/madler/crcany) library. It supports calculating CRC hashes of arbitary sizes as well as updating a crc hash over time. It takes advantage of crcany's ability to efficiently combine multiple CRCs to parallelize the CRC's calculation.

## Installation

`pip install anycrc`

## Usage

Use an existing model:

```python
>>> import anycrc
>>> crc32 = anycrc.Model('CRC32')
>>> crc32.calc(b'Hello World!')
472456355
```

Read the data in chunks:

```python
>>> crc32.update(b'Hello ')
3928882368
>>> crc32.update(b'World!')
472456355
```

The `update` method changes the internally stored CRC value, while `calc` doesn't. You can use `get` to retrieve the CRC value stored within the object:

```python
>>> crc32.get()
472456355
```

To specify the starting CRC value:

```python
>>> crc32.set(3928882368)
>>> crc32.calc('World!')
472456355
```

To go back to the initial value, use:

```python
>>> crc32.reset()
```

Specify your own CRC parameters:

```python
>>> # width, poly, init, refin, refout, xorout
>>> my_crc = anycrc.CRC(10, 0b0101010101, 0x3ff, True, False, 0)
>>> my_crc.calc('Hello World!')
35
```

The CRC will be computed in parallel when the input is large enough. If you want to disable parallelism, use:

```python
>>> anycrc.set_parallel(False)
```

For a list of pre-built models, check [models.py](https://github.com/marzooqy/anycrc/blob/main/src/anycrc/models.py). To get a list of the models at any time, use the following command:

`python -m anycrc models`

The maximum possible CRC width is 128 bits.

## Benchmarks

Calculating the CRC32 for lorem ipsum 1 million times:

| Module | Time Elapsed (s) | Speed (GiB/s) | Relative |
|---|:-:|:-:|:-:|
| anycrc | 0.19 | 2.14 | 1.00 |
| zlib | 0.28 | 1.49 | 1.44 |
| fastcrc | 0.80 | 0.52 | 4.12 |
| crcmod-plus | 0.91 | 0.46 | 4.71 |

Calculating the CRC32 for the text of lorem ipsum repeated 1 million times in a single pass:

| Module | Time Elapsed (s) | Speed (GiB/s) | Relative |
|---|:-:|:-:|:-:|
| anycrc (parallel) | 0.02 | 24.51 | 1.00 |
| anycrc (serial) | 0.13 | 3.21 | 7.64 |
| zlib | 0.22 | 1.93 | 12.72 |
| fastcrc | 0.67 | 0.62 | 39.75 |
| crcmod-plus | 0.67 | 0.62 | 39.73 |

Tested on a 10th generation Intel i7 processor. Parallel performance will depend on your system.

#### Notes

Parallelism is disabled when the length of the input data is under 200k, as the serial method is faster in that case.

The input needs to be very large in order to notice the speed advantage of the parallel algorithm.

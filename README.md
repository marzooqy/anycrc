This is a Cython module with bindings to the [crcany](https://github.com/madler/crcany) library. It supports calculating CRC hashes of arbitary sizes as well as updating a CRC hash over time.

## Installation

`pip install anycrc`

## Usage

Use an existing model:

```python
>>> import anycrc
>>> crc32 = anycrc.Model('CRC32-MPEG-2')
>>> crc32.calc(b'Hello World!')
2498069329
```

Create a CRC with specific parameters:

```python
>>> crc32 = anycrc.CRC(width=32, poly=0x04c11db7, init=0xffffffff, refin=False, refout=False, xorout=0x00000000)
>>> crc32.calc('Hello World!')
2498069329
```

Read the data in chunks:

```python
>>> value = crc32.calc(b'Hello ')
>>> crc32.calc(b'World!', value)
2498069329
```

The length of the data can be specified in bits by calling `calc_bits` and passing a [bitarray](https://github.com/ilanschnell/bitarray) object:

```python
>>> from bitarray import bitarray
>>> crc32 = anycrc.Model('CRC32-MPEG-2')
>>> bits = bitarray()
>>> bits.frombytes(b'Hello World!')
>>> value = crc32.calc_bits(bits[:50])
>>> crc32.calc_bits(bits[50:], value)
2498069329
```

To use bit lengths with reflected CRCs, create a little endian bitarray object: `bitarray(endian='little')`

To combine two CRCs, provide the CRC values along with the length of the second CRC's message in bytes:

```python
>>> crc32 = anycrc.Model('CRC32-MPEG-2')
>>> value = crc32.calc(b'Hello ')
>>> value2 = crc32.calc(b'World!')
>>> crc32.combine(value, value2, len(b'World!'))
2498069329
```

There is also a `combine_bits` method where the length argument is expected to be in bits.

For a list of pre-built models, check [models.py](https://github.com/marzooqy/anycrc/blob/main/src/anycrc/models.py). To get a list of the models at any time, use the following command:

`python -m anycrc models`

The maximum supported CRC width is 64 bits.

## Benchmark

| Module | Time (s) | Speed (MB/s) | Relative |
|---|:-:|:-:|:-:|
| anycrc | 0.04 | 2344.86 | x1.00 |
| fastcrc | 0.15 | 649.89 | x3.61 |
| crcmod-plus | 0.16 | 630.45 | x3.72 |
| crc-ct | 0.18 | 555.68 | x4.22 |
| libscrc | 0.67 | 148.48 | x15.79 |
| crcengine | 14.82 | 6.75 | x347.54 |
| pycrc | 29.34 | 3.41 | x688.07 |
| crccheck | 99.75 | 1.00 | x2339.08 |
| crc | 296.99 | 0.34 | x6964.11 |

Tested on a 10th generation Intel i7 processor.
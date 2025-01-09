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
| anycrc | 0.04 | 2562.95 | x1.00 |
| fastcrc | 0.15 | 648.86 | x3.95 |
| crcmod-plus | 0.16 | 642.37 | x3.99 |
| crc-ct | 0.18 | 561.70 | x4.56 |
| libscrc | 0.68 | 147.85 | x17.34 |
| crcengine | 14.33 | 6.98 | x367.33 |
| pycrc | 28.79 | 3.47 | x737.97 |
| crccheck | 98.00 | 1.02 | x2511.80 |
| crc | 298.26 | 0.34 | x7644.35 |

Tested on a 10th generation Intel i7 processor.

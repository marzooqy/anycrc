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
>>> bits = bitarray()
>>> bits.frombytes(b'Hello World!')
>>> value = crc32.calc_bits(bits[:50])
>>> crc32.calc_bits(bits[50:], value)
2498069329
```

To use bit lengths with reflected CRCs, create a little endian bitarray object: `bitarray(endian='little')`

To combine two CRCs, provide the CRC values along with the length of the second CRC's message in bytes:

```python
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

| Module | Speed (MiB/s) | Relative |
|---|:-:|:-:|
| anycrc | 2615.78 | x1.00 |
| crcmod-plus | 612.24 | x4.27 |
| fastcrc | 535.86 | x4.88 |
| libscrc | 193.20 | x13.54 |
| crcengine | 9.85 | x265.47 |
| pycrc | 8.31 | x314.77 |
| crccheck | 1.32 | x1975.24 |
| crc | 0.46 | x5673.25 |

Tested on a 12th generation Intel i7 processor.

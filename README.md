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

| Module | Speed (MB/s) | Relative | Notes |
|---|:-:|:-:|:-:|
| pycrc32 | 34559.57 | x0.08 | CRC32 only |
| crc32c | 30209.55 | x0.09 | CRC32C only |
| google-crc32c | 19348.59 | x0.15 | CRC32C only |
| zlib | 3186.10 | x0.89 | CRC32 only |
| anycrc | 2834.54 | x1.00 |  |
| fastcrc | 656.97 | x4.31 |  |
| crcmod-plus | 654.04 | x4.33 |  |
| crc-ct | 646.11 | x4.39 |  |
| libscrc | 215.91 | x13.13 |  |
| crcengine | 9.47 | x299.29 |  |
| pycrc | 7.55 | x375.39 |  |
| crccheck | 1.48 | x1921.18 |  |
| crc | 0.42 | x6670.14 |  |

Tested on a 12th generation Intel i7 processor.

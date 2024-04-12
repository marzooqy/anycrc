## crcany

The fastest Python CRC computation library available.

This is a Cython module with bindings to the [crcany](https://github.com/madler/crcany) library. It supports calculating CRC hashes of arbitary sizes as well as updating a crc hash over time.

## Installation

`pip install crcany`

## Usage

Use an existing model:

```python
>>> import crcany
>>> data = b'Hello World!'
>>> crc32 = crcany.Model('CRC32/ISO-HDLC')
>>> crc32.calc(data)
472456355
```

Read the data in chunks:

```python
>>> crc32.reset() #set to the initial value
>>> crc32.calc(b'Hello ')
3928882368
>>> crc32.calc(b'World!')
472456355
```

Specify your own CRC parameters:

```python
>>> # width, poly, init, refin, refout, xorout
>>> my_crc = crcany.CRC(10, 0b0101010101, 0x3ff, True, False, 0)
>>> my_crc.calc('Hello World!')
35
```

## Benchmarks

Calculating the CRC32 for lorem ipsum 10 million times:

```
crcany
Time Elapsed: 7.732s
Average: 0.773 us/run
Relative: 1.000

binascii
Time Elapsed: 7.612s
Average: 0.761 us/run
Relative: 0.984

fastcrc
Time Elapsed: 16.483s
Average: 1.648 us/run
Relative: 2.132

crcmod-plus
Time Elapsed: 18.259s
Average: 1.826 us/run
Relative: 2.361
```
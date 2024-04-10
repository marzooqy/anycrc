## Cyrc

Python CRC Calculation Library.

This an optimized Cython module based on the [pycrc](https://github.com/tpircher/pycrc) library. It supports calculating CRC hashes of arbitary sizes as well as updating a crc hash over time.

## Installation

`pip install cyrc`

## Usage

Use an existing model:

```python
>>> from cyrc.models import crc32
>>> data = b'Hello World!'
>>> val = crc32.init()
>>> val = crc32.update(val, data)
>>> crc32.finalize(val)
472456355
```

You can also read the data in chunks:

```python
>>> from cyrc.models import crc32
>>> val = crc32.init()
>>> val = crc32.update(val, b'Hello ')
>>> val = crc32.update(val, b'World!')
>>> val.finalize(val)
472456355
```

You can specify your own CRC parameters:

```python
>>> my_crc = cyrc.Model(
	width = 10,
	poly = 0b0101010101,
	reflect_in = True,
	xor_in = 0x3ff,
	reflect_out = False,
	xor_out = 0
	)
>>> val = my_crc.init()
>>> val = my_crc.update(val, data)
>>> my_crc.finalize(val)
35
```

If your input is a string, then it will be assumed to be utf-8 encoded. If your string has another encoding, then you should decode it first before passing it to the update method:

```python
string = 'Hello World!'
b = string.encode('ascii')
```

## Benchmarks

Calculating the CRC32 for lorem impsum 10 million times:

```
cyrc
Time Elapsed: 27.403s
Average: 2.740 us/run
Relative: 1.000

binascii
Time Elapsed: 7.469s
Average: 0.747 us/run
Relative: 0.273

fastcrc
Time Elapsed: 16.235s
Average: 1.624 us/run
Relative: 0.592

crcmod-plus
Time Elapsed: 19.285s
Average: 1.928 us/run
Relative: 0.704
```

1- The standard library module [binascii](https://docs.python.org/3/library/binascii.html) is the fastest, however it only provides two CRC functions.

2- [fastcrc](https://github.com/overcat/fastcrc) is the fastest library, however it's limited to a specific number of pre-built functions and doesn't allow you to specify your own parameters.

3- [crcmod-plus](https://github.com/ntamas/crcmod-plus) offers a good balance between performance and customizability.

4- cyrc is the slowest of the four, however it's the most customizable.
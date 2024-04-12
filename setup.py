from setuptools import setup, Extension

setup(
    name = 'crcany',
    version = '0.2.1',
    ext_modules = [Extension(name='crcany.crcany', sources=['src/crcany/crcany.pyx', 'lib/crcany/model.c', 'lib/crcany/crc.c'])]
)
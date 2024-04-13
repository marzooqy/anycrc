from setuptools import setup, Extension

setup(
    name = 'anycrc',
    version = '0.2.2',
    ext_modules = [Extension(name='anycrc.anycrc', sources=['src/anycrc/anycrc.pyx', 'lib/crcany/model.c', 'lib/crcany/crc.c'])]
)
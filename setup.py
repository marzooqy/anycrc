from setuptools import setup, Extension

setup(
    name = 'anycrc',
    version = '0.3.1',
    package_dir = {"": "src"},
    ext_modules = [Extension(name='anycrc.anycrc', sources=['src/anycrc/anycrc.pyx', 'lib/crcany/model.c', 'lib/crcany/crc.c'])]
)
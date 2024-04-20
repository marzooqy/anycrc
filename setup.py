from setuptools import setup, Extension
from Cython.Build import cythonize
setup(
    name = 'anycrc',
    version = '0.3.3',
    package_dir = {"": "src"},
    ext_modules = cythonize([Extension(name='anycrc.anycrc', sources=['src/anycrc/anycrc.pyx', 'lib/crcany/model.c', 'lib/crcany/crc.c'])])
)
from setuptools import setup, Extension
from Cython.Build import cythonize

setup(
    name = 'anycrc',
    version = '0.4.0',
    package_dir = {"": "src"},
    
    ext_modules = [
        Extension(
            name='anycrc.anycrc',
            extra_compile_args=['-openmp', '-fopenmp'],
            sources=['src/anycrc/anycrc.pyx', 'lib/crcany/model.c', 'lib/crcany/crc.c']
        )
    ]
)

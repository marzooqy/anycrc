from setuptools import setup, Extension
import sys

compile_args = [] 

if sys.platform == 'win32':
    compile_args = ['-openmp']
else:
    compile_args = ['-fopenmp']
    
setup(
    name = 'anycrc',
    version = '0.4.0',
    package_dir = {"": "src"},
    
    ext_modules = [
        Extension(
            name='anycrc.anycrc',
            extra_compile_args=compile_args,
            sources=['src/anycrc/anycrc.pyx', 'lib/crcany/model.c', 'lib/crcany/crc.c']
        )
    ]
)
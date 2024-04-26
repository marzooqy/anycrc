from setuptools import setup, Extension
import sys

compile_args = ['-fopenmp', '-O2']
link_args = ['-fopenmp', '-O2']

if sys.platform == 'win32':
    link_args = ['-static-libgcc',
                 '-static-libstdc++',
                 '-Wl,-Bstatic,--whole-archive',
                 '-lwinpthread',
                 '-Wl,--no-whole-archive',
                 '-fopenmp',
                 '-O2']
                
setup(
    name = 'anycrc',
    version = '0.6.0',
    package_dir = {"": "src"},
    
    ext_modules = [
        Extension(
            name='anycrc.anycrc',
            extra_compile_args=compile_args,
            extra_link_args=link_args,
            sources=['src/anycrc/anycrc.pyx', 'lib/crcany/model.c', 'lib/crcany/crc.c']
        )
    ]
)
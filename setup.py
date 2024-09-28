from setuptools import setup, Extension
import sys

if sys.platform == 'win32':
    compile_args = ['/O2']
    link_args = ['/O2']
else:
    compile_args = ['-O2']
    link_args = ['-O2']

setup(
    name = 'anycrc',
    version = '1.0.1',
    package_dir = {"": "src"},
    ext_modules = [
        Extension(
            name='anycrc.anycrc',
            extra_compile_args=compile_args,
            extra_link_args=link_args,
            sources=['src/anycrc/anycrc.pyx',
                     'lib/crcany/model.c',
                     'lib/crcany/crc.c']
        )
    ]
)

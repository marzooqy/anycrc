from setuptools.command.build_ext import build_ext
from setuptools import setup, Extension
import sys

if sys.platform == 'win32':
    compile_args = ['-DWINDOWS_EXPORT', '/O2']
else:
    compile_args = ['-O2']

#This is a hack to make the build system treat the shared library as a Python C extension
#so we can then use cibuildwheel to build the C library in different operating systems
#https://github.com/himbeles/ctypes-example/blob/master/setup.py

class custom_build_ext(build_ext):
    def get_export_symbols(self, ext):
        return ext.export_symbols

    def get_ext_filename(self, ext_name):
        if sys.platform == 'win32':
            return ext_name + '.dll'
        else:
            return ext_name + '.so'

setup(
    ext_modules=[
        Extension(
            name='anycrc.crcany',
            extra_compile_args=compile_args,
            sources=['lib/crcany/model.c',
                     'lib/crcany/crc.c']
        )
    ],
    cmdclass={'build_ext': custom_build_ext}
)
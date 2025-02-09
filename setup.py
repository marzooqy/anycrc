from setuptools.command.build_ext import build_ext
from setuptools.command.bdist_wheel import bdist_wheel
from setuptools import setup, Extension
import sys

if sys.platform == 'win32':
    compile_args = ['-DWINDOWS_EXPORT']
else:
    compile_args = []

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

#Rename the resulting wheel file
class custom_bdist_wheel(bdist_wheel):
    def get_tag(self):
        _, _, platform = super().get_tag()
        return 'py3', 'none', platform

setup(
    ext_modules=[
        Extension(
            name='anycrc.crcany',
            extra_compile_args=compile_args,
            sources=['lib/crcany/model.c',
                     'lib/crcany/crc.c']
        )
    ],
    cmdclass={'build_ext': custom_build_ext,
              'bdist_wheel': custom_bdist_wheel}
)
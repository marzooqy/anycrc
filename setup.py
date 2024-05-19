from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys

is_64bit = sys.maxsize > 2 ** 32

class Build(build_ext):
    def finalize_options(self):
        super().finalize_options()
        
        #gcc optimizes the slice-by-16 algorithm better
        if sys.platform == 'win32' and is_64bit:
            self.compiler = "mingw32"
            
if sys.platform == 'win32':
    if is_64bit:
        compile_args = ['-fopenmp', '-O2','-DMS_WIN64']
        link_args = ['-static-libgcc',
                     '-static-libstdc++',
                     '-Wl,-Bstatic,--whole-archive',
                     '-lwinpthread',
                     '-Wl,--no-whole-archive',
                     '-fopenmp',
                     '-O2',
                     '-DMS_WIN64']
                     
    else:
        #github actions and python make it difficult to compile with 32-bit MinGW, so just use MSVC
        compile_args = ['/openmp']
        link_args = []
        
else:
    compile_args = ['-fopenmp', '-O2']
    link_args = ['-fopenmp', '-O2']
    
setup(
    name = 'anycrc',
    version = '0.7.2',
    package_dir = {"": "src"},
    cmdclass={"build_ext": Build},
    ext_modules = [
        Extension(
            name='anycrc.anycrc',
            extra_compile_args=compile_args,
            extra_link_args=link_args,
            sources=['src/anycrc/anycrc.pyx', 'lib/crcany/model.c', 'lib/crcany/crc.c', 'lib/crcany/crcdbl.c']
        )
    ]
)
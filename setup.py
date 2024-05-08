from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys

class Build(build_ext):
    def finalize_options(self):
        super().finalize_options()
        
        #gcc optimizes the slice-by-16 algorithm better
        if sys.platform == 'win32':
            self.compiler = "mingw32"
            
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
    version = '0.6.1',
    package_dir = {"": "src"},
    cmdclass={"build_ext": Build},
    ext_modules = [
        Extension(
            name='anycrc.anycrc',
            extra_compile_args=compile_args,
            extra_link_args=link_args,
            sources=['src/anycrc/anycrc.pyx', 'lib/crcany/model.c', 'lib/crcany/crc.c']
        )
    ]
)
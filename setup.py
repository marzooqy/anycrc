from setuptools import setup, Extension

setup(
    name = 'anycrc',
    version = '1.3.2',
    ext_modules = [
        Extension(
            name='anycrc.crc',
            sources=['src/anycrc/crc.pyx',
                     'lib/crcany/model.c',
                     'lib/crcany/crc.c']
        )
    ],
    package_data={'': ['lib']}
)
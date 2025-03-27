from setuptools import setup, Extension

setup(
    ext_modules = [
        Extension(
            name='anycrc.crc',
            sources=['src/anycrc/crc.pyx',
                     'lib/crcany/model.c',
                     'lib/crcany/crc.c']
        )
    ]
)
from setuptools import setup, Extension

setup(
    name = 'cyrc',
    version = '0.1.1',
    ext_modules = [Extension(name='cyrc.cyrc', sources=['src/cyrc/cyrc.pyx'])]
)
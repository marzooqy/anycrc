from setuptools import setup, Extension
import platform
import sys
import sysconfig

macros = []
use_limited_api = False
options = {}

#https://github.com/pypa/cibuildwheel/blob/main/cibuildwheel/architecture.py
if (platform.machine().lower() not in ('x86_64', 'amd64', 'aarch64', 'arm64', 'arm64_v8a')) or sys.maxsize < 2 ** 32:
    macros.append(('DISABLE_SIMD', None))

if sys.version_info.minor >= 11 and not sysconfig.get_config_var("Py_GIL_DISABLED"):
    use_limited_api = True
    macros.append(('Py_LIMITED_API', 0x030B0000))
    options = {'bdist_wheel': {'py_limited_api': 'cp311'}}

setup(
    ext_modules = [
        Extension(
            name='anycrc.crc',
            sources=['src/anycrc/crc.pyx',
                     'lib/crc-clmul/crc.c',
                     'lib/crc-clmul/cpu.c'],
            define_macros = macros,
            py_limited_api = use_limited_api
        )
    ],
    options = options
)
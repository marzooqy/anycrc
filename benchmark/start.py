# This script creates a virtual environment, installs all dependancies, and runs the benchmark
import shutil
import subprocess
import sys

modules = ['pycrc32', 'crc32c', 'google-crc32c', 'fastcrc', 'crcmod-plus', 'crc-ct', 'libscrc', 'crcengine', 'pycrc', 'crccheck']

if len(sys.argv) > 1 and sys.argv[1] == '--local':
    modules.append('..')
else:
    modules.append('anycrc')

# Use Python 3.12 because pycrc32 and google-crc32c lack a 3.13 build
subprocess.run(['py', '-3.12', '-m' 'venv', '.venv'])

try:
    subprocess.run(['.venv/Scripts/pip', 'install'] + modules)

    # This is necessary due to a naming conflict between crc and crc-ct
    shutil.move('.venv/Lib/site-packages/crc', '.venv/Lib/site-packages/crc_ct')
    subprocess.run(['.venv/Scripts/pip', 'install', 'crc'])

    subprocess.run(['.venv/Scripts/python', 'bench.py'])

except KeyboardInterrupt:
    pass

shutil.rmtree('.venv')
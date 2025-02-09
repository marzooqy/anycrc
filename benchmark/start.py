# This script creates a virtual environment, installs all dependancies, and runs the benchmark
try:
    import os
    import shutil
    import subprocess
    import sys

    # Use Python 3.12 because pycrc32 and google-crc32c lack a 3.13 build
    if sys.version_info[0] != 3 or sys.version_info[1] != 12:
        print('Expected Python 3.12 with the Python Launcher')
        sys.exit()

    if os.path.isdir('.venv/Lib/site-packages'):
        for file in os.listdir('.venv/Lib/site-packages'):
            if file == 'crc' or file.startswith('crc-') or file == 'crc_ct' or file.startswith('crc_ct-'):
                shutil.rmtree(os.path.join('.venv/Lib/site-packages', file))

    modules = ['..', 'pycrc32', 'crc32c', 'google-crc32c', 'fastcrc', 'crcmod-plus', 'crc-ct', 'libscrc', 'crcengine', 'pycrc', 'crccheck']

    subprocess.run(['py', '-3.12', '-m' 'venv', '.venv'])
    subprocess.run(['.venv/Scripts/pip', 'install'] + modules)

    # This is necessary due to a naming conflict between crc and crc-ct
    shutil.move('.venv/Lib/site-packages/crc', '.venv/Lib/site-packages/crc_ct')
    subprocess.run(['.venv/Scripts/pip', 'install', 'crc'])

    subprocess.run(['.venv/Scripts/python', 'bench.py'])

except KeyboardInterrupt:
    pass
# This script creates a virtual environment, installs all dependancies, and runs the benchmark
try:
    import shutil
    import subprocess
    import os
    import sys

    # Use Python 3.12 because pycrc32 and google-crc32c lack a 3.13 build
    if sys.version_info[0] != 3 or sys.version_info[1] != 12:
        print('Expected Python 3.12 with the Python Launcher')
        sys.exit()

    packages_path = '.venv/Lib/site-packages'

    if os.path.isdir(packages_path):
        for folder in os.listdir(packages_path):
            if folder == 'crc' or folder.startswith('crc-') or folder == 'crc_ct' or folder.startswith('crc_ct-'):
                shutil.rmtree(os.path.join(packages_path, folder))

    modules = ['pycrc32', 'crc32c', 'google-crc32c', 'fastcrc', 'crcmod-plus', 'crc-ct', 'libscrc', 'crcengine', 'pycrc', 'crccheck']

    if len(sys.argv) > 1 and sys.argv[1] == '-local':
        modules.append('..')
    else:
        modules.append('anycrc')

    subprocess.run(['py', '-3.12', '-m' 'venv', '.venv'])
    subprocess.run(['.venv/Scripts/pip', 'install'] + modules)

    # This is necessary due to a naming conflict between crc and crc-ct
    shutil.move('.venv/Lib/site-packages/crc', '.venv/Lib/site-packages/crc_ct')
    subprocess.run(['.venv/Scripts/pip', 'install', 'crc'])

    subprocess.run(['.venv/Scripts/python', 'bench.py'])

except KeyboardInterrupt:
    pass
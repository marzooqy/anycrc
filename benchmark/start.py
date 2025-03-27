# This script creates a virtual environment, installs all dependancies, and runs the benchmark
try:
    import os
    import shutil
    import subprocess
    import sys

    packages_path = '.venv/Lib/site-packages'

    if os.path.isdir(packages_path):
        for file in os.listdir(packages_path):
            if file == 'crc' or file.startswith('crc-') or file == 'crc_ct' or file.startswith('crc_ct-'):
                shutil.rmtree(os.path.join(packages_path, file))

    modules = ['fastcrc', 'crcmod-plus', 'crc-ct', 'libscrc', 'crcengine', 'pycrc', 'crccheck']

    subprocess.run(['python', '-m' 'venv', '.venv'])
    subprocess.run(['.venv/Scripts/pip', 'install'] + modules)

    # This is necessary due to a naming conflict between crc and crc-ct
    shutil.move(os.path.join(packages_path, 'crc'), os.path.join(packages_path, 'crc_ct'))
    subprocess.run(['.venv/Scripts/pip', 'install', 'crc'])
    
    subprocess.run(['.venv/Scripts/pip', 'install', '..'])

    subprocess.run(['.venv/Scripts/python', 'bench.py'])

except KeyboardInterrupt:
    pass
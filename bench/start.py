# This script creates a virtual environment, installs all dependancies, and runs the benchmark
try:
    import subprocess

    modules = ['anycrc', 'fastcrc', 'crcmod-plus', 'crcengine', 'pycrc', 'crccheck', 'crc']

    subprocess.run(['python', '-m' 'venv', '.venv'])
    subprocess.run(['.venv/Scripts/pip', 'install'] + modules)
    subprocess.run(['.venv/Scripts/python', 'bench.py'])

except KeyboardInterrupt:
    pass
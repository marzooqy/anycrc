:: This script creates a virtual environment, installs all dependancies, and runs the benchmark
python -m venv ".venv"
".venv/Scripts/pip" install .. fastcrc crcmod-plus crc-ct libscrc crcengine pycrc crccheck

:: This is necessary due to a naming conflict between crc and crc-ct
move ".venv/Lib/site-packages/crc" ".venv/Lib/site-packages/crcct"
".venv/Scripts/pip" install crc

".venv/Scripts/python" bench.py
rd /s /q ".venv"
pause
# This script creates a virtual environment and runs the benchmark
# This is necessary due to a naming conflict between crc and crc-ct
python -m venv ".venv"
".venv/Scripts/pip" install anycrc fastcrc crcmod-plus crc-ct crcengine pycrc libscrc
mv ".venv/Lib/site-packages/crc" ".venv/Lib/site-packages/crcct"
".venv/Scripts/pip" install crc
".venv/Scripts/python" bench.py
rm -rf ".venv"
sleep 60
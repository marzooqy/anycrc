#This script is a part of the build system
#It renames the wheels built with cibuildwheel so that they can be used in different versions of Python

import os

for file_name in os.listdir('wheelhouse'):
    if file_name.endswith('.whl'):
        parts = file_name.split('-')
        parts[2] = 'cp3'
        parts[3] = 'none'
        
        new_file_name = parts[0]
        for part in parts[1:]:
            new_file_name += '-' + part
            
        os.rename(os.path.join('wheelhouse', file_name),
                  os.path.join('wheelhouse', new_file_name))
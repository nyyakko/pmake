import os
import sys
import shutil

BUILD_TYPE = sys.argv[1] if len(sys.argv) >= 2 else 'debug'

if (os.path.isdir('build/')):
    shutil.rmtree('build/')

os.system(f'cmake -DCMAKE_BUILD_TYPE={BUILD_TYPE} -G Ninja -B./build/ -S./')

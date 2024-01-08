import os
import sys

BUILD_TYPE = sys.argv[1] if len(sys.argv) >= 2 else 'debug'

os.system(f'cmake -DCMAKE_BUILD_TYPE={BUILD_TYPE} -G Ninja -B./build/ -S./')

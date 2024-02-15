import os
import sys

sys.argv.pop(0)

os.system(f'cmake { " ".join(sys.argv) } -G Ninja -B./build/ -S./')

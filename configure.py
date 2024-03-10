import os
import sys

sys.argv.pop(0)

os.system('vcpkg install --feature-flags=registries,manifests')
os.system(f'cmake --preset debug { " ".join(sys.argv) }')

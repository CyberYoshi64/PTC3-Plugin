#!/usr/bin/python3

import os, sys
from glob import glob

cwd = os.getcwd()

os.chdir(os.path.join(cwd, "Resources", "images"))

images = glob("*.png")

os.chdir(os.path.join(cwd, "Resources", "utilities"))
assert os.path.exists("png2c.py")

toolPath = os.path.join(cwd, "Resources", "utilities", "png2c.py")

os.chdir(os.path.join(cwd, "Library", "source", "CTRPluginFrameworkImpl", "Graphics", "Icons"))

for i in images:
    img = os.path.join(cwd, "Resources", "images", i)
    name = i[:-4]
    print(i)
    os.system(f"\"{toolPath}\" \"{img}\" {name}.c {name}")
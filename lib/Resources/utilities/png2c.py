#!/usr/bin/python3

# Created by PabloMK7, modified by CyberYoshi64
from PIL import Image
import sys

imorg = Image.open(sys.argv[1])
outf = sys.argv[2] if len(sys.argv)>2 else sys.argv[1].replace(".png", ".c")
name = sys.argv[1][:sys.argv[1].rfind(".png")] if len(sys.argv)<4 else sys.argv[3]
im = imorg.transpose(Image.ROTATE_270)
imorg.close()
pixels = im.convert('RGBA')
pix = list(pixels.getdata())

with open(outf, "w", encoding="utf-8") as f:
	f.write("static const unsigned char %s_array[%d] = {" % (name, len(pix * 4)))
	i = 0
	for p in pix:
		if i>0: f.write(", ")
		if not (i % 3):
			f.write("\n    ")
		f.write("0x%02X, 0x%02X, 0x%02X, 0x%02X" % (int(p[3]),int(p[2]),int(p[1]),int(p[0])))
		i += 1
	f.write("\n};\n\n")
	f.write(f"const int {name}_array_length = {len(pix)*4};\nunsigned char *{name} = (unsigned char*){name}_array;\n")

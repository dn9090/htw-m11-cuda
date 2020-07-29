import os
import sys
from PIL import Image, ImageChops

if len(sys.argv) < 4:
	print('usage: %s <first file> <second file> <output file>\n' % (sys.argv[0]))
	exit(1)


a = Image.open(sys.argv[1]).convert('RGB')
b = Image.open(sys.argv[2]).convert('RGB')

diff = ImageChops.difference(a, b)
diff.save(sys.argv[3])

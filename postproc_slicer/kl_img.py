#! /usr/bin/python

#
# Kikai Labs Post Processor (c) by Kikai Labs
#
# Kikai Labs Post Processor is licensed under a
# Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
#
# You should have received a copy of the license along with this
# work.  If not, see <http://creativecommons.org/licenses/by-nc-sa/3.0/>.
#

import os
import tempfile
import struct
from PIL import Image

# Writes a gci file with the given width, height and data. Data is pixel data in a bidimensional
# array where each pixel is an array with R,G and B values (in that order).
def write_gci(width, height, data, writer):
	width_data = struct.pack('<H', width)
	height_data = struct.pack('<H', height)
	colors_data = struct.pack('<H', 16) # this is 0x16 (16 colors) + 0x00 ("image", not video)
	writer.write(width_data)
	writer.write(height_data)
	writer.write(colors_data)
	
	for y in range(0,height):
		for x in range(0,width):
			px = data[x,y]
			pxgci = ((px[0] & 0xF8) << 8)  | ((px[1] & 0xFC) << 3) | ((px[2] & 0xF8) >> 3)
			writer.write(struct.pack('B', pxgci / 256))
			writer.write(struct.pack('B', pxgci % 256))
	# Fill the rest with zeros (in sectors of 512 bytes)
	writen_so_far = width*height*2+6
	remaining_to_write = ((writen_so_far + 511)/512)*512 - writen_so_far
	for r in range(0, remaining_to_write):
		writer.write('\0')

def read_gci(reader):
	data = reader.read()
	
	width_data = data[0:2]
	height_data = data[2:4]
	
	width = struct.unpack_from('<H', width_data)[0]
	height = struct.unpack_from('<H', height_data)[0]
	limit=width*height*2+6
	im = Image.new("RGB", (width, height), "white")
	pix = im.load()
	
	for y in range(0,height):
		for x in range(0,width):
			p = (y*width+x)*2+6
			pxgci = struct.unpack_from('>H', data[p:p+2])[0]
			r=0
			g=0
			b=0
			r = (pxgci >> 8) & 0xF8
			g = (pxgci >> 3) & 0xFC
			b = (pxgci << 3) & 0xF8
			pix[x,y] = (r, g, b)
			
	return im

# Generates a gci file from a BMP file.
def gci2bmp(gci_path, img_path):
	gci_reader = open(gci_path, mode="rb")
	im = read_gci(gci_reader)
	gci_reader.close()
	
	pix = im.load()
	
	im.save(img_path)

# Generates a gci file from a BMP file.
def bmp2gci(bmp_path, klt_path):
	im = Image.open(bmp_path)
	pix = im.load()
	
	out_file = open(klt_path, "wb")
	write_gci(im.size[0], im.size[1], pix, out_file)
	out_file.close()

# This method saves the given image to the destination file. If the file is gci, it automatically does what is necessary to store it with that extension.
def saveImage(im, out):
	if out.endswith(".gci"):
		s = im.size
		with open(out, "wb") as f:
			write_gci(s[0], s[1], im.load(), f)
	else:
		im.save(out)

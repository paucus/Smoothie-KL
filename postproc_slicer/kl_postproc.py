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
import sys
import tarfile
import math
import kl_img
from VTKRender import Render
from GcodeParser import GcodeParser
from PIL import Image

# dialogs
#import Tkinter as tk
#from tkFileDialog import askopenfilename

WORK_WIDTH=1280
WORK_HEIGHT=960
THUMB_WIDTH=128
THUMB_HEIGHT=96
PREVIEW_WIDTH=320
PREVIEW_HEIGHT=240
RENDER_WIDTH=640
RENDER_HEIGHT=480

KL_METADATA_EXTENSION=".k3d"


def gen_preview(model_path, img_path, formats, width, height, draw_reference_planes):
	# The layer height parameter doesn't matter in this case. Set anything. In this case, 0.1.
	render = Render(width, height, False, draw_reference_planes, 0.1)
	
	render.add_model(model_path)
	
	print "Rendering"
	render.finish()
	
	for f in formats:
		#print "Saving %s" % f
		#kl_img.saveImage(renwin, img_path + "." + f) # Disable to test
		render.save(img_path + "." + f)
	

def archive_files(tar_file, img_dir, files):
	tout = tarfile.TarFile(tar_file, "w")
	for fname in files:
		tout.add(os.path.join(img_dir, fname), arcname=fname)
	tout.close()

def del_tmp_dir(img_dir, files):
	for fname in files:
		os.remove(os.path.join(img_dir, fname))
	# Now delete the folder. It should be empty at this point.
	os.rmdir(img_dir)

def gen_gcode_preview(gcode_path, img_path, width, height, draw_reference_planes):
	print "Parsing G-Code"
	parser = GcodeParser(gcode_path)
	curves = parser.get_curves()
	print "Rendering"
	render = Render(width, height, False, draw_reference_planes, parser.get_layer_height())
	render.set_print_volume(parser.bounding_box())
	render.add_curves(curves)
	render.finish()
	render.save(img_path)
	
def gen_preview_from_render(src_img, img_path, formats, out_width, out_height):
	im = Image.open(src_img)
	im.thumbnail((out_width, out_height), Image.ANTIALIAS)
	for f in formats:
		kl_img.saveImage(im, img_path + "." + f)


# There are two ways to use this script:
# * Specifying only the .gcode file: In this case, the script will search for a file with .stl or
# .objextension in the same directory, and will generate a .tar file with a thumbnail and a
# full-screen preview.
# If no model file is found, it will prompt for one using a dialog.
# * Specifying only the .stl/.obj file: In this case
# * Specifying model, image and dimensions: In this case it will generate an image from the given
# model and dimensions.
if __name__ == "__main__":
	print "Kikai Labs Slicing Post-Processor"
	print "There is NO WARRANTY, to the extent permitted by law. Read the License file for more information."
	if (len(sys.argv) < 2) or ((len(sys.argv) == 2 or len(sys.argv) == 5) and not sys.argv[1].endswith((".gcode",".stl",".obj"))):
		print "params: <gcode/stl/obj> | <stl/obj> <jpg/png/bmp/gci> <width> <height>"
		sys.exit(128) # invalid args
	fin = sys.argv[1]
	if (len(sys.argv) == 2): # gcode/stl/obj only
		print "File: %s" % fin
		
		prefix = fin[0:fin.rfind(".")]
		
		img_dir = tempfile.mkdtemp("", "kl_postproc_")
		if (fin.endswith(".gcode")):
			gen_gcode_preview(fin, os.path.join(img_dir, "work.bmp"), WORK_WIDTH, WORK_HEIGHT, draw_reference_planes = True)
		else:
			gen_preview(fin, os.path.join(img_dir, "work"), ["bmp"], WORK_WIDTH, WORK_HEIGHT, draw_reference_planes = True)
		gen_preview_from_render(os.path.join(img_dir, "work.bmp"), os.path.join(img_dir, "thumb"), ("png", "gci"), THUMB_WIDTH, THUMB_HEIGHT)
		gen_preview_from_render(os.path.join(img_dir, "work.bmp"), os.path.join(img_dir, "preview"), ("jpg", "gci"), PREVIEW_WIDTH, PREVIEW_HEIGHT)
		gen_preview_from_render(os.path.join(img_dir, "work.bmp"), os.path.join(img_dir, "render"), ["jpg"], RENDER_WIDTH, RENDER_HEIGHT)
		print "Saving k3d"
		archive_files(prefix + KL_METADATA_EXTENSION, img_dir, ("thumb.gci", "thumb.png", "preview.gci", "preview.jpg", "render.jpg"))
		del_tmp_dir(img_dir, ("work.bmp", "thumb.gci", "thumb.png", "preview.gci", "preview.jpg", "render.jpg"))
	else: # stl/obj
		out_fname = sys.argv[2]
		out_slices = out_fname.split(".")
		ext = out_slices[-1]
		out_prefix = ".".join(out_slices[0:-1])
		gen_preview(fin, out_prefix, [ext], int(sys.argv[3]), int(sys.argv[4]), draw_reference_planes = False)
	
	print "Process finished"


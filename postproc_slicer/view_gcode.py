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

import sys
from VTKRender import Render
import time
from GcodeParser import GcodeParser

PREVIEW_WIDTH=1280
PREVIEW_HEIGHT=960


def gcode2img(fin, img_path, width, height):
	t0 = time.time()
	print "paso 1 %f" % 0
	print "paso 2 %f" % (time.time() - t0)
	
	parser = GcodeParser(fin)
	curves = parser.get_curves()
	
	render = Render(width, height, False, True, parser.get_layer_height())
	print "paso 3 %f" % (time.time() - t0)
	
	render.set_print_volume(parser.bounding_box())
	
	print "paso 3.4 %f" % (time.time() - t0)
	
	render.add_curves(curves)
	
	print "paso 3.7 %f" % (time.time() - t0)
	
	render.finish()
	print "paso 4 %f" % (time.time() - t0)
	
	render.save(img_path)
	print "paso 5 %f" % (time.time() - t0)
	

if __name__ == "__main__":
	if (len(sys.argv) < 3):
		print "params: <gcode> <jpg/png/gci>"
		sys.exit(128) # invalid args
	fin = sys.argv[1]
	img_path = sys.argv[2]
	gcode2img(fin, img_path, PREVIEW_WIDTH, PREVIEW_HEIGHT)


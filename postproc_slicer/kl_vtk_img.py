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

# This script uses the VTK library.
 
import vtk
import os
import kl_img

def instanceWriter(img_path):
	if (img_path.lower().endswith(".png")):
		return vtk.vtkPNGWriter()
	elif (img_path.lower().endswith(".jpg")):
		return vtk.vtkJPEGWriter()
	elif (img_path.lower().endswith(".bmp")):
		return vtk.vtkBMPWriter()
	else:
		raise ValueError('Unknown output format')

def saveImage(renwin, img_path):
	wif = vtk.vtkWindowToImageFilter()
	wif.SetInput(renwin)
	wif.Update()
	
	if (img_path.lower().endswith(".gci")):
		# We must make a temp file for this and generate the tmp later
		imgwri = instanceWriter(".bmp")
		imgwri.SetFileName(os.path.join(tempfile.gettempdir(), "post.bmp"))
	else:
		imgwri = instanceWriter(img_path)
		imgwri.SetFileName(img_path)
	imgwri.SetInputConnection(wif.GetOutputPort())
	imgwri.Write()
	
	if (img_path.lower().endswith(".gci")):
		# Now, post process the image
		kl_img.bmp2gci(os.path.join(tempfile.gettempdir(), "post.bmp"), img_path)



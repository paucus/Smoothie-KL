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

import vtk
import kl_vtk_img
import math
import sys

ROT_Z=45
ROT_ELEVATION=22.5

class Render:
	def __init__(self, width, height, interactive, draw_reference_planes, layer_height):
		self.interactive = interactive
		self.draw_reference_planes = draw_reference_planes
		self.width = width
		self.height = height
		self.bounds = []
		self.last_percentage = -1
		self.tubes_radius = layer_height / 2
		
		#if not self.interactive:
			#gf = vtk.vtkGraphicsFactory()
			#gf.SetUseMesaClasses(True)
			#imf = vtk.vtkImagingFactory() # Disable to test
			#imf.SetUseMesaClasses(True)
	
		self.ren = vtk.vtkRenderer()
		self.renwin = vtk.vtkRenderWindow()
		self.renwin.SetSize(self.width, self.height)

		if not self.interactive:
			self.renwin.SetOffScreenRendering(True) # Disable to test
			#self.renwin.SetAAFrames(5)              # Disable to test
		self.renwin.AddRenderer(self.ren)
		
		if self.interactive:
			self.rwi = vtk.vtkRenderWindowInteractor()
			self.rwi.SetInteractorStyle(vtk.vtkInteractorStyleTrackballCamera())  # Enable to test
			self.rwi.SetRenderWindow(self.renwin)


	def finish(self):

		self.ren.SetBackground(1, 1, 1)
		
		if not self.interactive:
			wif = vtk.vtkWindowToImageFilter()
			wif.SetInput(self.renwin)
			wif.Update()
		
		self.ren.ResetCamera()
		cam = self.ren.GetActiveCamera()
		cam.Elevation(-90)
		
		# Update view up, as rotating 90 degrees of elevation makes view and up be parallel
		cam.OrthogonalizeViewUp()
		self.ren.ResetCamera()
		cam.Azimuth(ROT_Z)
		self.ren.ResetCamera()
		cam.Elevation(ROT_ELEVATION)
		
		cam.Dolly(1.0)
		self.ren.ResetCameraClippingRange()
		
		# Add Grid
		# We mustn't do this before, or otherwise the scale will be adapted to the grid too
		# I know there are better ways to do this, but I couldn't find good documentation about it.
		if self.draw_reference_planes:
			bounds = self.get_bounds()
			planeactor = self.add_plane(bounds, 'x', min_side=True)
			self.ren.AddActor(planeactor)
			planeactor =self. add_plane(bounds, 'y', min_side=False)
			self.ren.AddActor(planeactor)
			planeactor = self.add_plane(bounds, 'z', min_side=True)
			self.ren.AddActor(planeactor)
		
		# Render
		self.renwin.Render()
		if self.interactive:
			self.rwi.Start()  # Enable to test
	
	def set_print_volume(self, bbox):
		self.bounds = bbox
	
	# Saves the rendered image to the specified file
	def save(self, img_path):
		kl_vtk_img.saveImage(self.renwin, img_path) 


	# Every time an actor that must be included in the scene is added, this
	# method must be called to ensure it fits in the camera view.
	def add_actor(self, actor):
		self.ren.AddActor(actor)

	# Adds the specified model file (stl, obj) to the scene
	def add_model(self, model_path):
		actor = self.__polydata_to_actor(self.__load_model_polydata(model_path))
		self.add_actor(actor)
	
	# Adds a polyline to the scene, giving it a radius of tubes_radius.
	def add_curve(self, c):
		actor = self.__tubes_to_actor(self.__curve_to_tubes(c))
		self.add_actor(actor)
			
	def add_curves(self, cs):
		actor = self.__tubes_to_actor(self.__curves_to_tubes(cs))
		self.add_actor(actor)
	
	# Returns the bounds of the whole scene
	def get_bounds(self):
		return self.bounds
		


	def __tubes_to_actor(self, tube):
		mapper = vtk.vtkPolyDataMapper()
		mapper.SetInputConnection(tube.GetOutputPort())

		actor = vtk.vtkActor()
		actor.SetMapper(mapper)
		actor.GetProperty().SetColor(1.0, 0.55, 0.0)
		return actor

	def progressCallback(self, caller, p):
		perc = int((caller.GetProgress()+0.004)*100)
		if perc / 5 > self.last_percentage / 5:
			print "Progress: %d%%\r" % perc,
			self.last_percentage = perc
			sys.stdout.flush()
		if caller.GetProgress() == 1.0:
			print "Progress: 100% Done!"
			self.last_percentage = -1

	def __curve_to_tubes(self, c):
		pd = vtk.vtkPolyData()
		
		ps = vtk.vtkPoints()
		for p in c:
			ps.InsertNextPoint(p)
		pd.SetPoints(ps)
		
		ca = vtk.vtkCellArray()
		
		for i in range(1,len(c)):
			l = vtk.vtkLine()
			l.GetPointIds().SetId(0,i-1)
			l.GetPointIds().SetId(1,i)
			ca.InsertNextCell(l)
		
		pd.SetLines(ca)
		
		# Make a tube for each line
		tf = vtk.vtkTubeFilter()
		if vtk.VTK_MAJOR_VERSION <= 5:
			tf.SetInput(pd)
		else:
			tf.SetInputData(pd)
		tf.SetNumberOfSides(16)
		tf.SetRadius(self.tubes_radius)
		#tf.AddObserver(vtk.vtkCommand.ProgressEvent, self.progressCallback)
		return tf
	
	def __curves_to_tubes(self, cs):
		pd = vtk.vtkPolyData()
		
		ps = vtk.vtkPoints()
		for c in cs:
			for p in c:
				ps.InsertNextPoint(p)
		pd.SetPoints(ps)
		
		ca = vtk.vtkCellArray()
		
		acum = 0
		for c in cs:
			for i in range(1,len(c)):
				l = vtk.vtkLine()
				l.GetPointIds().SetId(0,acum+i-1)
				l.GetPointIds().SetId(1,acum+i)
				ca.InsertNextCell(l)
			acum += len(c)
		
		pd.SetLines(ca)
		
		# Make a tube for each line
		tf = vtk.vtkTubeFilter()
		if vtk.VTK_MAJOR_VERSION <= 5:
			tf.SetInput(pd)
		else:
			tf.SetInputData(pd)
		tf.SetNumberOfSides(8)
		tf.SetRadius(self.tubes_radius)
		#tf.AddObserver(vtk.vtkCommand.ProgressEvent, self.progressCallback)
		return tf
	
	#def __extend_bounding_box(self, bounds):
		#if len(self.bounds) == 0:
			#self.bounds = list(bounds)
		#else:
			#for r in range(0,6,2):
				#if bounds[r] < self.bounds[r]:
					#self.bounds[r] = bounds[r]
				#if bounds[r+1] > self.bounds[r+1]:
					#self.bounds[r+1] = bounds[r+1]
	

	def add_plane(self, bounds, plane, min_side):
		boxsize = math.ceil(max(bounds[1]-bounds[0], bounds[3]-bounds[2], bounds[5]-bounds[4]))
		#boxsize = int(math.ceil((bounds[5]-bounds[4]) / 2))
		pts = vtk.vtkPoints()
		cx = (bounds[1] + bounds[0])/2
		cy = (bounds[3] + bounds[2])/2
		cz = boxsize + bounds[4] # (bounds[5] + bounds[4])/2
		
		# rearr expects first the plane (ex: for 'z' plane, the z height), then the fixed value for
		# that row, then the value that represents the ends of the segment.
		if plane == 'x':
			rearr = lambda v: (v[0], v[1], v[2])
		elif plane == 'y':
			rearr = lambda v: (v[1], v[0], v[2])
		else:
			rearr = lambda v: (v[2], v[1], v[0])
		
		c = rearr((cx, cy, cz))
		if (min_side):
			msc = -1
		else:
			msc = 1
		
		for i in range(-3, 4):
			pts.InsertNextPoint(rearr((c[0] + msc * boxsize, c[1] + (boxsize * i) / 3, c[2] - boxsize)))
			pts.InsertNextPoint(rearr((c[0] + msc * boxsize, c[1] + (boxsize * i) / 3, c[2] + boxsize)))
		for i in range(-3, 4):
			pts.InsertNextPoint(rearr((c[0] + msc * boxsize, c[1] - boxsize, c[2] + (boxsize * i) / 3)))
			pts.InsertNextPoint(rearr((c[0] + msc * boxsize, c[1] + boxsize, c[2] + (boxsize * i) / 3)))
		
		lines = vtk.vtkCellArray()
		for j in range(0, 14, 2):
			line = vtk.vtkLine()
			line.GetPointIds().SetId(0,j)
			line.GetPointIds().SetId(1,j+1)
			lines.InsertNextCell(line)
		for j in range(14, 28, 2):
			line = vtk.vtkLine()
			line.GetPointIds().SetId(0,j)
			line.GetPointIds().SetId(1,j+1)
			lines.InsertNextCell(line)
		pd = vtk.vtkPolyData()
		pd.SetPoints(pts)
		pd.SetLines(lines)
		planemapper = vtk.vtkPolyDataMapper()
		if vtk.VTK_MAJOR_VERSION <= 5:
			planemapper.SetInput(pd)
		else:
			planemapper.SetInputData(pd)
		planeactor = vtk.vtkActor()
		planeactor.SetMapper(planemapper)
		planeactor.GetProperty().SetColor(0.75, 0.75, 0.75)
		planeactor.GetProperty().SetLineWidth(5)
		return planeactor

	def __instance_model_reader(self, model_path):
		if (model_path.lower().endswith(".stl")):
			return vtk.vtkSTLReader()
		elif (model_path.lower().endswith(".obj")):
			return vtk.vtkOBJReader()
		else:
			raise ValueError('Unknown input format')

	def __load_model_polydata(self, model_path):
		reader = self.__instance_model_reader(model_path)
		reader.SetFileName(model_path)
		reader.Update()
		polydata = reader.GetOutput()
		return polydata

	def __polydata_to_actor(self, polydata):
		mapper = vtk.vtkPolyDataMapper()
		if vtk.VTK_MAJOR_VERSION <= 5:
			mapper.SetInput(polydata)
		else:
			mapper.SetInputData(polydata)
		actor = vtk.vtkActor()
		actor.SetMapper(mapper)
		actor.GetProperty().SetColor(1.0, 0.55, 0.0)
		return actor


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
import math
import time

from OpenGL.GL import *
from OpenGL.GL.ARB.multisample import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
from OpenGL.GLE import *
from OpenGL.GL.framebufferobjects import *
from PIL import Image

OUTINIT_X=10000
OUTINIT_Y=10000
#LIGHT_POS1 = (-10.0, -10.0, 10.0, 10.0)
#LIGHT_DIR1 = (0, 1, -1, 0)
LIGHT_POS1 = (0.0, -1000.0, 1000.0, 1.0)
LIGHT_DIR1 = (0, 1, -1, 0)
LIGHT_POS2 = (-1000.0, 0.0, 1000.0, 1.0)
LIGHT_DIR2 = (1, 0, -1, 0)
BACK_COLOR = (1.0, 1.0, 1.0)
LAYER_HEIGHT_FACTOR=3.5
TUBE_FACES=8

CAM_POS = (0, -20, 0)
SCALE=-4.5
ROT_Z=-45
ROT_ELEVATION=22.5


#MAT_COLOR = (0.3, 0.17, 0.0)
MAT_COLOR = (0.3, 0.17, 0.0, 1.0)
#MAT_COLOR = (1.0, 0.55, 0.0)
LIGHT_WHITE = (0.7, 0.7, 0.7, 1.0)
DIM_WHITE = (-0.3, -0.3, -0.3, 1.0)
BLACK = (-1.0, -1.0, -1.0, 1.0)

USE_RENDER_BUFFER=False
TEX_MSAA=False

class Render:
	def __init__(self, width, height, interactive, draw_reference_planes, layer_height):
		self.width = width
		self.height = height
		self.offset = (0,0,0)
		self.scale = 1.0
		self.tubes_radius = layer_height * LAYER_HEIGHT_FACTOR
		
		# Init the glut window
		glutInit(sys.argv)
		if TEX_MSAA:
			# Set Anti-aliasing MSAA samples
			glutSetOption(GLUT_MULTISAMPLE, 8);
		glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH)# | GLUT_MULTISAMPLE)
		
		# Enable anti aliasing
		if TEX_MSAA:
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_MULTISAMPLE_ARB);
			#glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
		
			#iMultiSample = glGetIntegerv(GL_SAMPLE_BUFFERS)
			#print "iMultiSample = %d" % iMultiSample
			#iNumSamples = glGetIntegerv(GL_SAMPLES)
			#print "iNumSamples = %d" % iNumSamples
		
		glutInitWindowSize(1, 1)
		glutInitWindowPosition(OUTINIT_X, OUTINIT_Y)
		glutCreateWindow("hello")
		glutHideWindow()


		# Init Depth Buffer
		fbo_depth = glGenRenderbuffers(1)
		glBindRenderbuffer(GL_RENDERBUFFER, fbo_depth)
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, self.width, self.height)
		glBindRenderbuffer(GL_RENDERBUFFER, 0)
		
		# The target of our render. It can be either a texture or a render buffer
		if USE_RENDER_BUFFER:
			# Init Render Buffer
			fbo_color = glGenRenderbuffers(1)
			glBindRenderbuffer(GL_RENDERBUFFER, fbo_color)
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, self.width, self.height)
			glBindRenderbuffer(GL_RENDERBUFFER, 0)
		else:
			# Init Frame Buffer Texture
			glEnable(GL_MULTISAMPLE)
			tex = glGenTextures(1)
			if TEX_MSAA:
				glBindTextureMultisample(GL_TEXTURE_2D_MULTISAMPLE, tex)
				AA_samples = 5
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, AA_samples, GL_RGBA4, self.width, self.height, False)
			else:
				glBindTexture(GL_TEXTURE_2D, tex)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0)
				#glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
				#glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
			
			
			
		
		# Init Frame Buffer
		fbo = glGenFramebuffers(1)
		glBindFramebuffer(GL_FRAMEBUFFER, fbo)
		if USE_RENDER_BUFFER:
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fbo_color)
		else:
			# Bind texture to frame buffer
			if TEX_MSAA:
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0)
			else:
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0)
			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE):
				print "SOMETHING FAILED DURING TEXTURE CREATION"
			
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo_depth)
		# DO NOT unbind the frame buffer, or otherwise it won't draw the objects there
		#glBindFramebuffer(GL_FRAMEBUFFER, 0)
		
		
		## Configure OpenGL Scene
		glEnable(GL_LIGHTING)
		glEnable(GL_LIGHT0)
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0); # Omnidirectional
		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 2.0)
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0)
		glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0)
		glEnable(GL_LIGHT1)
		glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 180.0); # Omnidirectional
		glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 4.0)
		glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.0)
		glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0)
		
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE)
		glShadeModel(GL_SMOOTH)
		# Cull face doesn't seem to work well with plePolyCylinder
		#glEnable(GL_CULL_FACE)
		glEnable(GL_DEPTH_TEST)
		
		# Select Background color
		glClearColor (BACK_COLOR[0], BACK_COLOR[1], BACK_COLOR[2], 0.0)
		
		# Setup GLE
		gleSetNumSides(TUBE_FACES)
		# Default parameters lead to horrible results!
		gleSetJoinStyle(TUBE_CONTOUR_CLOSED|TUBE_JN_RAW)

		# Initialize Viewport
		h = float(height) / float(width);
		glViewport(0, 0, width, height)
		glMatrixMode(GL_PROJECTION)
		glLoadIdentity()
		R = 1
		R2 = 3.85
		glFrustum(-R, R, -R*h, R*h, R2, 10000.0)
		glRotatef(90, 1, 0, 0)
		glMatrixMode(GL_MODELVIEW)
		glLoadIdentity()

		# clear all pixels 
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

		
		
		
		


	def set_print_volume(self, bbox):
		self.offset = (-(bbox[0]+bbox[1])/2, -(bbox[2]+bbox[3])/2, -(bbox[4]+bbox[5])/2)
		self.scale = SCALE/max((bbox[1]-bbox[0]), (bbox[3]-bbox[2]), (bbox[5]-bbox[4]))
		
		glMatrixMode(GL_MODELVIEW)
		glTranslatef(CAM_POS[0], CAM_POS[1], CAM_POS[2])
		glScalef(self.scale, self.scale, self.scale)
		
		glRotatef(ROT_ELEVATION, 1, 0, 0)
		glRotatef(-ROT_Z, 0, 0, 1)
		#glRotatef(0, 0, 1, 0)
		
		glTranslatef(self.offset[0], self.offset[1], self.offset[2])
		
		#glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, DIM_WHITE)
		#glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, LIGHT_WHITE)
		#glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, BLACK)
		
		glLightfv(GL_LIGHT0, GL_AMBIENT, DIM_WHITE)
		glLightfv(GL_LIGHT0, GL_DIFFUSE, LIGHT_WHITE)
		glLightfv(GL_LIGHT0, GL_POSITION, LIGHT_POS1)
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, LIGHT_DIR1)
		glLightfv(GL_LIGHT1, GL_AMBIENT, DIM_WHITE)
		glLightfv(GL_LIGHT1, GL_DIFFUSE, LIGHT_WHITE)
		glLightfv(GL_LIGHT1, GL_POSITION, LIGHT_POS2)
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, LIGHT_DIR2)

	def finish(self):
		glFlush()


	def save(self, out):
		buff = ( GLubyte * (3*self.width*self.height) )(0)
		data = glReadPixels(0, 0, self.width, self.height, GL_RGB, GL_UNSIGNED_BYTE, buff)
		
		im = Image.new("RGB", (self.width, self.height), "white")
		outdata = im.load()
		
		i = 0
		for y in xrange(0, self.height):
			for x in xrange(0, self.width):

				outdata[x, self.height-y-1] = (data[i], data[i+1], data[i+2])
				i += 3
		
		im.save(out)
		
	
	
	def add_curves(self, cs):
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MAT_COLOR)
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MAT_COLOR)
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, BLACK)
		for c in cs:
			self.add_curve(c)
	
	def dist(self,v1,v2):
		a = 0
		for i in xrange(0, 3):
			a += (v1[i]-v2[i])*(v1[i]-v2[i])
		return math.sqrt(a)
	#def subs_dist(self, v1,v2):
		#RECT_SUBDIVSION_LEN=0.1
		#return max(1, int(self.dist(v1, v2) / RECT_SUBDIVSION_LEN))
	#def escal(self, ka,a,kb,b):
		#return [ka*a[i]+kb*b[i] for i in xrange(0,3)]
	def sub(self,a, b):
		return [a[i]-b[i] for i in xrange(0,3)]
	def norm(self,v):
		a = 0
		for i in xrange(0, 3):
			a += v[i]*v[i]
		return math.sqrt(a)
	def prod(self,v1,v2):
		a = 0
		for i in xrange(0, 3):
			a += v1[i]*v2[i]
		return a
	# plePolyCylinder doesn't draw the extremes of the polyline. They just mark the direction
	def extend_curve(self, p0, p1):
		return [ 2*p1[i] - p0[i] for i in range(0, 3) ]
	
	def add_directional_caps(self, curve):
		if len(curve) < 2:
			return []
		else:
			# Too short. Just extend the curve to let glePolyCylinder draw it
			c2 = [self.extend_curve(curve[1], curve[0])]
			c2.extend(curve)
			c2.append(self.extend_curve(curve[-2], curve[-1]))
			return [ c2 ]
			
	def add_curve(self, curve):
		cps = self.add_directional_caps(curve)
		for c in cps:
			if len(c) < 4:
				None
			else:
				# It has the same color, so, it is not necessary to repeat it N times.
				# http://linas.org/gle/tube.html
				glePolyCylinder(c, None, self.tubes_radius)


if __name__ == "__main__":
	if len(sys.argv) < 2:
		print "params: <img_out>"
		sys.exit(128)
	
	render = Render(1024, 768, False, True)
	

	PI=3.14159265358979323	
	#s=time.time()
	N=100
	R=1
	c1=math.cos((N-1)*2*PI/N)*R
	s1=math.sin((N-1)*2*PI/N)*R
	c = []
	for i in xrange(0, N+1):
		c2 = c1
		s2 = s1
		c1 = math.cos((i*2*PI)/N)*R
		s1 = math.sin((i*2*PI)/N)*R
		c.append((c1, s1, -1.5))
	#print time.time() - s
	
	render.add_curve(c)
	
	render.finish()
	
	render.save(sys.argv[1])
	
	

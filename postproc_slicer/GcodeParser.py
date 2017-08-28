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

import copy
import operator

DEFAULT_LAYER_HEIGHT=0.2
# Ignore layer heights that represent less than 5%
OCCURRENCE_FACTOR=0.05
RESOLUTION=100.0

# This class reads a .gcode file and calculates a set of curves that represent the extrusion path.
# So far this parser does not comply with the "Stardard G-code", as it doesn't recognize valid
# commands like "G1X0Y0", "G 1 X 0 Y 0", or " G1 X0 Y0" (all equivalent to "G1 X0 Y0") and many
# other cases.
# It doesn't accept relative positioning, only absolute. It also only accepts one line height for
# the whole g-code. If more than one is found, the greatest one is used.
class GcodeParser:
	def __init__(self, f):
		self.f = f
		self.bbox = []
		self.layer_height_histogram = {}
		self.layer_height = 0.0
		self.last_z = 0.0
	
	def get_curves(self):
		lines = load_gcode_lines(self.f)
		return self.__parse_curves(lines)
	
	def bounding_box(self):
		return self.bbox
	def get_layer_height(self):
		return self.layer_height
	
	def __increase_counter_layer_height(self, dz):
		# Sove resolution issues by rounding to the 2nd digit
		dz = round(dz*RESOLUTION) / RESOLUTION
		if dz <= 0:
			None
		elif not self.layer_height_histogram.has_key(dz):
			self.layer_height_histogram[dz] = 1
		else:
			self.layer_height_histogram[dz] += 1
	
	def __parse_curves(self, gcode_lines):
		self.layer_height_histogram = {}
		self.bbox = []
		curves = []
		current_curve = []
		current_pos = [0,0,0]
		last_e = 0.0
		for line in gcode_lines:
			try:
				g = parse_gcode(line)
				if is_movement_gcode(g):
					if has_extrusion(g, last_e):
						set_pos(g, current_pos)
						if current_pos[2] != self.last_z:
							self.__increase_counter_layer_height(current_pos[2] - self.last_z)
							self.last_z = current_pos[2]
						ps = gcode_to_points(g, current_pos)
						self.extend_without_duplicates(current_curve, ps)
					else:
						if len(current_curve) > 1:
							# We haven't considered the first point of the curve for the
							# bounding box calculation (see the comment a few lines below).
							# This is the moment to do this.
							if not self.is_bbox_initialized():
								self.init_bbox(current_pos)
							self.extend_bbox(current_curve[0])
							curves.append(current_curve)
						
						set_pos(g, current_pos)
						# at this stage we don't know whether this point will be the beginning
						# of an extrusion movement, so, we can't use it for the bounding box
						# calculation. the moment to do this is a few lines above, when
						# current_curve is added to the curves array.
						current_curve = [copy.copy(current_pos)]
					if g.has_key("E"):
						last_e = g["E"]
				elif is_reset_e_gcode(g):
					last_e = g["E"]
			except ValueError as e:
				print line
				print e

		g = parse_gcode(line)
		
		if len(current_curve) > 1:
			# Consider the first point of the curve for bounding box calculation.
			if not self.is_bbox_initialized():
				self.init_bbox(current_pos)
			self.extend_bbox(current_curve[0])
			curves.append(current_curve)
		
		self.analyze_layer_height_histogram_and_get_layer_height()
		
		return curves

	def analyze_layer_height_histogram_and_get_layer_height(self):
		if len(self.layer_height_histogram) == 0:
			print "No layer height detected. Using %f." % DEFAULT_LAYER_HEIGHT
			# No layer heights, return a default one.
			self.layer_height = DEFAULT_LAYER_HEIGHT
		elif len(self.layer_height_histogram) == 1:
			# This is the typical case. Only one layer height.
			self.layer_height = self.layer_height_histogram.keys()[0]
		else:
			# Multiple options, we must choose among them which is the best.
			# First order by occurrence. Discard the least frequent (<5%) and work with the rest.
			# The list of heights is expected to be short.
			# List of height sorted by occurrence.
			temp_lh = sorted(self.layer_height_histogram.items(), key=operator.itemgetter(1), reverse=True)
			total = sum(self.layer_height_histogram.values())
			# Ignore infrequent ones.
			min_occurrence = int(total * OCCURRENCE_FACTOR)
			temp_lh2 = [a for a in temp_lh if a[1] > min_occurrence]
			
			# Let's analyze again the results
			if len(temp_lh2) == 0:
				# Mhh. Strange. No layer height has more than 5% of the occurrences. This is a
				# patological case. Use the most frequest one.
				self.layer_height = temp_lh[0][0]
			else:
				# Many valid layer heights. Keep the greatest one. For layers with thin heights there
				# will be overlapping, which is not that terrible.
				self.layer_height = 0.0
				for a in temp_lh2:
					if a[0] > self.layer_height:
						self.layer_height = a[0]
				#print "Multiple Layer Heights. Using %f." % self.layer_height

	def extend_without_duplicates(self, c, ps):
		if not self.is_bbox_initialized() and len(ps) > 0:
			self.init_bbox(ps[0])
		for p in ps:
			if len(c) == 0 or p != c[-1]:
				c.append(p)
				self.extend_bbox(p)
	
	def init_bbox(self, p0):
		self.bbox = [p0[0], p0[0], p0[1], p0[1], p0[2], p0[2]]
	def is_bbox_initialized(self):
		return len(self.bbox) == 6
	def extend_bbox(self, p):
		if self.bbox[0] > p[0]:
			self.bbox[0] = p[0]
		if self.bbox[1] < p[0]:
			self.bbox[1] = p[0]
		if self.bbox[2] > p[1]:
			self.bbox[2] = p[1]
		if self.bbox[3] < p[1]:
			self.bbox[3] = p[1]
		if self.bbox[4] > p[2]:
			self.bbox[4] = p[2]
		if self.bbox[5] < p[2]:
			self.bbox[5] = p[2]


def load_gcode_lines(path):
	with open(path) as f:
		return f.readlines()

def gcode_to_points(g, pos):
	# Now it only supports G1/0 which are movments, so, only one element is returned in the list
	cur_pos = copy.copy(pos)
	return [ cur_pos ]

def remove_comments(line):
	comment = line.find(";")
	if comment >= 0:
		return line[0:comment]
	else:
		return line

def parse_gcode(line):
	# Ignore non-g codes (so far we don't need M codes either)
	if len(line) == 0 or line[0] != 'G':
		return {};
	# Remove comments and extra spaces
	line = remove_comments(line).strip()

	parts = line.split(' ')
	
	g = {}
	for p in parts:
		if len(p) > 0:
			letter = p[0]
			value = float(p[1:])
			g[letter] = value
	return g

def is_movement_gcode(g):
	try:
		return g.has_key('G') and (g["G"] == 1 or g["G"] == 0) and (g.has_key("X") or g.has_key("Y") or g.has_key("Z"))
	except:
		return False

def is_reset_e_gcode(g):
	try:
		return g.has_key("G") and g["G"] == 92 and g.has_key("E")
	except Exception as e:
		print e
		return False
	
def has_extrusion(g, last_e):
	return g.has_key("E") and g["E"] > last_e

def set_pos(g, current_pos):
	if g.has_key("X"):
		current_pos[0] = g["X"]
	if g.has_key("Y"):
		current_pos[1] = g["Y"]
	if g.has_key("Z"):
		current_pos[2] = g["Z"]






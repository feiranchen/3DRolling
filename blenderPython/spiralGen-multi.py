import re, sys, getopt, math, os, shutil
from xml.dom import minidom
from array import *

class SegmentMeta(object):
	def __init__(self, ID, total_length):
		file_id = ID
		self.segment_total_length = total_length

def linkFaces(objfile, vertex_group_count):
	line = 1
	objfile.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n"
		.format(1, 3, 4, 2))
	objfile.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n"
		.format(4*vertex_group_count,
			4*vertex_group_count - 1,
			4*vertex_group_count - 3,
			4*vertex_group_count - 2))
	for i in range(vertex_group_count - 1):
		# # outer
		objfile.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n".format(line, line+1, line+5, line+4))
		# # inner
		objfile.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n".format(line+2, line+6, line+7, line+3))
		# at 0
		objfile.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n".format(line, line+4, line+6, line+2))
		# at height
		objfile.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n".format(line+1, line+3, line+7, line+5))
		line += 4

# All units should be in mm
def spiralGen(filename, argv):
	# Params(all units are in mm)
	target_radius = 100 # 10cm
	target_gap = 0.5 # .5mm
	init_radius = 2 # inital rod radius 2mm
	# Get customized parameters
	print '''usage: python spiralGen-multi.py -r <target_radius> -g <target_gap> -i <inital_radius> #all units in mm.
	Default values are used if not provided'''
	try:
		opts,args = getopt.getopt(argv,"r:g:i:")
		for opt, arg in opts:
			if opt == '-r':
				target_radius = float(arg)
			elif opt == '-g':
				target_gap = float(arg)
			elif opt == '-i':
				init_radius = float(arg)
	except getopt.GetoptError:
		print "no options received, using default values"
	print "target_radius = %f" % target_radius
	print "target_gap = %f" % target_gap
	print "init_radius = %f" % init_radius

	height = target_radius * 2 # a reasonable height
	max_slice_width = 0.6
	cutoff_length = 600 # 10mm is typically a resonable length
	file_count = 0
	layer_thickness = target_gap / 10
	uv_height = (height + 0.0)/cutoff_length

	# States
	radius = init_radius
	slice_num = 10
	vertex_group_count = 0
	outer_curr_lenth = 0

	# Global Variables
	shutil.rmtree("gen")
	os.makedirs("gen")
	objfile = open("gen/" + filename + "_" + str(file_count) +".obj", 'w')
	segments = []
	while radius <= target_radius:
		# adjust the number of slices for each full turn of the spiral
		while (2 * math.pi * radius / slice_num > max_slice_width):
			slice_num+=1
		d_radius = target_gap / slice_num
		for i in range(slice_num):
			theta = i * (2 * math.pi / slice_num)
			inner_radius = radius - layer_thickness
			# Vertices and UV mapping
			objfile.write("v {0} {1} {2}\n"
				.format(-height/2, math.sin(theta) * radius, math.cos(theta) * radius))
			objfile.write("v {0} {1} {2}\n"
				.format(height/2, math.sin(theta) * radius, math.cos(theta) * radius))
			objfile.write("v {0} {1} {2}\n"
				.format(-height/2, math.sin(theta) * inner_radius, math.cos(theta) * inner_radius))
			objfile.write("v {0} {1} {2}\n"
				.format(height/2, math.sin(theta) * inner_radius, math.cos(theta) * inner_radius))
			objfile.write("vt {0} {1}\n"
				.format(0, outer_curr_lenth/cutoff_length))
			objfile.write("vt {0} {1}\n"
				.format(uv_height, outer_curr_lenth/cutoff_length))
			objfile.write("vt {0} {1}\n"
				.format(0, outer_curr_lenth/cutoff_length))
			objfile.write("vt {0} {1}\n"
				.format(uv_height, outer_curr_lenth/cutoff_length))

			radius += d_radius
			vertex_group_count += 1
			if radius <= target_radius:
				slice_length = (radius + d_radius/2) * math.sqrt(math.pow(math.sin(theta) - math.sin(theta + 2 * math.pi / slice_num), 2) +
					math.pow(math.cos(theta) - math.cos(theta + 2 * math.pi / slice_num), 2))
				outer_curr_lenth += slice_length
				if outer_curr_lenth > cutoff_length:
					theta_sliced = theta + (2 * math.pi / slice_num) * ((slice_length - (outer_curr_lenth - cutoff_length))/slice_length)
					# Add Cutoff veritices and Vertices and UV mapping
					objfile.write("v {0} {1} {2}\n".format(-height/2, math.sin(theta_sliced) * radius, math.cos(theta_sliced) * radius))
					objfile.write("v {0} {1} {2}\n".format(height/2, math.sin(theta_sliced) * radius, math.cos(theta_sliced) * radius))
					objfile.write("v {0} {1} {2}\n".format(-height/2, math.sin(theta_sliced) * inner_radius, math.cos(theta_sliced) * inner_radius))
					objfile.write("v {0} {1} {2}\n".format(height/2, math.sin(theta_sliced) * inner_radius, math.cos(theta_sliced) * inner_radius))
					objfile.write("vt {0} {1}\n".format(0, 1))
					objfile.write("vt {0} {1}\n".format(uv_height, 1))
					objfile.write("vt {0} {1}\n".format(0, 1))
					objfile.write("vt {0} {1}\n".format(uv_height, 1))
					linkFaces(objfile, vertex_group_count + 1)
					objfile.close()
					segments.append(SegmentMeta(ID = file_count, total_length = cutoff_length))
					file_count += 1
					objfile = open("gen/" + filename + "_" + str(file_count) +".obj", 'w')
					objfile.write("v {0} {1} {2}\n".format(-height/2, math.sin(theta_sliced) * radius, math.cos(theta_sliced) * radius))
					objfile.write("v {0} {1} {2}\n".format(height/2, math.sin(theta_sliced) * radius, math.cos(theta_sliced) * radius))
					objfile.write("v {0} {1} {2}\n".format(-height/2, math.sin(theta_sliced) * inner_radius, math.cos(theta_sliced) * inner_radius))
					objfile.write("v {0} {1} {2}\n".format(height/2, math.sin(theta_sliced) * inner_radius, math.cos(theta_sliced) * inner_radius))
					objfile.write("vt {0} {1}\n".format(0, 0))
					objfile.write("vt {0} {1}\n".format(uv_height, 0))
					objfile.write("vt {0} {1}\n".format(0, 0))
					objfile.write("vt {0} {1}\n".format(uv_height, 0))
					outer_curr_lenth -= cutoff_length
					vertex_group_count = 1
			else:
				linkFaces(objfile, vertex_group_count + 1)
				objfile.close()
				break
		sys.stdout.write("\rsprial growing to radius @ %fmm" % radius)
		sys.stdout.flush()
	print "\nTotal number of slices %d" % slice_num

def main(argv):
	spiralGen("test", argv)

if __name__ == "__main__":
    main(sys.argv[1:])
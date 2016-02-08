import re
from xml.dom import minidom
import math

# All units should be in mm
def spiralGen(filename):
	obj = open(filename+".obj", 'w')
	target_radius = 100 # 10cm
	target_gap = 0.5 # .5mm
	height = target_radius * 2 # a reasonable height
	max_slice_width = 0.6

	init_radius = 2 # inital rod radius 2mm

	layer_thickness = target_gap / 10

	# States
	radius = init_radius
	slice_num = 10
	vertex_group_count = 0
	outer_total_lenth = 0

	while radius <= target_radius:
		while (2 * math.pi * radius / slice_num > max_slice_width):
			slice_num+=1
		d_radius = target_gap / slice_num
		for i in range(slice_num):
			theta = i * (2 * math.pi / slice_num)
			inner_radius = radius - layer_thickness
			obj.write("v {0} {1} {2}\n".format(-height/2, math.sin(theta) * radius, math.cos(theta) * radius))
			obj.write("v {0} {1} {2}\n".format(height/2, math.sin(theta) * radius, math.cos(theta) * radius))
			obj.write("v {0} {1} {2}\n".format(-height/2, math.sin(theta) * inner_radius, math.cos(theta) * inner_radius))
			obj.write("v {0} {1} {2}\n".format(height/2, math.sin(theta) * inner_radius, math.cos(theta) * inner_radius))
			radius += d_radius
			vertex_group_count += 1
			if radius <= target_radius:
				outer_total_lenth += (radius + d_radius/2) * math.sqrt(math.pow(math.sin(theta) - math.sin(theta + 2 * math.pi / slice_num), 2) + 
					math.pow(math.cos(theta) - math.cos(theta + 2 * math.pi / slice_num), 2))
		print radius
	print slice_num
	# Reset State and assign UV map
	radius = init_radius
	slice_num = 10
	uv_height = height/outer_total_lenth
	uv_curr_lenth = 0
	while radius <= target_radius:
		inner_radius = radius - layer_thickness
		while (2 * math.pi * radius / slice_num > max_slice_width):
			slice_num+=1
		d_radius = target_gap / slice_num
		for i in range(slice_num):
			theta = i * (2 * math.pi / slice_num)
			obj.write("vt {0} {1}\n".format(0, uv_curr_lenth/outer_total_lenth))
			obj.write("vt {0} {1}\n".format(uv_height, uv_curr_lenth/outer_total_lenth))
			obj.write("vt {0} {1}\n".format(0, uv_curr_lenth/outer_total_lenth))
			obj.write("vt {0} {1}\n".format(uv_height, uv_curr_lenth/outer_total_lenth))
			radius += d_radius
			uv_curr_lenth += (radius + d_radius/2) * math.sqrt(math.pow(math.sin(theta) - math.sin(theta + 2 * math.pi / slice_num), 2) + 
					math.pow(math.cos(theta) - math.cos(theta + 2 * math.pi / slice_num), 2))
	print slice_num
	print outer_total_lenth

	line = 1
	obj.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n".format(1, 3, 4, 2))
	obj.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n".format(4*vertex_group_count, 4*vertex_group_count-1, 4*vertex_group_count-3, 4*vertex_group_count-2))
	for i in range(vertex_group_count - 1):
		# # outer
		obj.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n".format(line, line+1, line+5, line+4))
		# # inner
		obj.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n".format(line+2, line+6, line+7, line+3))
		# at 0
		obj.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n".format(line, line+4, line+6, line+2))
		# at height
		obj.write("f {0}/{0} {1}/{1} {2}/{2} {3}/{3}\n".format(line+1, line+3, line+7, line+5))
		line += 4
	
spiralGen("test")
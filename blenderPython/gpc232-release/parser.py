import re
from xml.dom import minidom

def file_to_svg(filename):
	f = open(filename, 'r')
	svg = open(filename+".svg", 'w')
    
	svg.write('<?xml version="1.0" standalone="no"?>\n' +
'<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" \n' +
'  "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">\n' +
'<svg width="32768" height="60.73" viewBox="0 0 32768 60.73"\n' +
'     xmlns="http://www.w3.org/2000/svg" version="1.1">\n')
	polygons = int(f.readline())

	for i in range(polygons):
		vertexs = int(f.readline())
		ishole = int(f.readline())
		print vertexs
		points_str = ""
		if ishole == 1:
			for j in range(vertexs):
				pair = str(f.readline()).split()
				points_str += pair[0] + ',' + pair[1] + " "
			svg.write('<polygon stroke="black" stroke-width="0.01" fill="rgb(6, 204, 14)" fill-opacity="0.25" points="' + points_str + '" />\n');
		else:
			first_pair_str = ""
			for j in range(vertexs):
				pair = str(f.readline()).split()
				if (j == 0):
					first_pair_str = pair[0] + ',' + pair[1] + " "
				points_str += pair[0] + ',' + pair[1] + " "
			points_str += first_pair_str
			svg.write('<polygon class="hole" stroke="black" stroke-width="0.01" fill="rgb(6, 204, 14)" fill-opacity="0.25" points="' + points_str + '" />\n');

	svg.write('</svg>\n')

def svg_to_bare(filename):
	doc = minidom.parse(filename + ".svg")
	out = open(filename+".out", 'w')
	poly = doc.getElementsByTagName("polygon")
	polylines = doc.getElementsByTagName("polyline")

	out.write(str(len(poly)) + '\r\n')
	for p in poly:
		if p.getAttribute("display") != "none":
			# print p.getAttribute("points")
			out.write(str(len(p.getAttribute("points").split(" ")) - 1) + '\r\n')
			out.write('1\r\n')
			arr = p.getAttribute("points").split(" ")
			for i in range(len(arr)):
				if (i<len(arr) -1):
					pair = arr[i].split(",")
					out.write(pair[0] + ' ' + pair[1] + '\r\n')
	# polygons.extend([Polyline(points_string=p.getAttribute("points")) for p in polylines if p.getAttribute("display") != "none"])

def svg_to_svg(filename):
	doc = minidom.parse(filename + ".svg")
	svg = open(filename+".out.svg", 'w')
	poly = doc.getElementsByTagName("polygon")
	polylines = doc.getElementsByTagName("polyline")

	svg.write('<?xml version="1.0" standalone="no"?>\n' +
'<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" \n' +
'  "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">\n' +
'<svg width="32768" height="32768" viewBox="0 0 32768 32768"\n' +
'     xmlns="http://www.w3.org/2000/svg" version="1.1">\n')

	for p in poly:
		if p.getAttribute("display") != "none":
			# print p.getAttribute("points")
			arr = p.getAttribute("points").split(" ")
			svg.write('<polygon stroke="black" stroke-width="1" fill="rgb(6, 204, 14)" fill-opacity="0.25" points="')
			for i in range(len(arr)):
				if (i<len(arr) -1):
					pair = arr[i].split(",")
					svg.write(pair[1] + ',' + pair[0] + ' ')
			svg.write('" />\n')
	svg.write('</svg>\n')

# flip the orientation of the image
svg_to_svg("blender_uv_output_0")
# <filename>.out is in the form that gpc can read
svg_to_bare("blender_uv_output_0.out")
#file_to_svg("subjfile")
#file_to_svg("outfile")

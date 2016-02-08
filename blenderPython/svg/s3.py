import re
from xml.dom import minidom


class Point(object):
    def __init__(self, p, pl):
        self.x = float(p[0])
        self.y = float(p[1])
        self.pl = pl

    def __repr__(self):
        return str(self.x) + "," + str(self.y)

    def __str__(self):
        return str(self.x) + "," + str(self.y)
        
    def __lt__(self, other):
        return self.x < other.x

    def btw(self, x0, x1):
        return x0 < self.x < x1

    @staticmethod
    def intersect(p0, p1, x0, x1):
        # given two points, p0 and p1
        # and two x values, x0 and x1
        # return point of intersection between either x values and the line connecting the points

        if p0.x < x0 < p1.x or p1.x < x0 < p0.x:
            x = x0
        elif p0.x < x1 < p1.x or p1.x < x1 < p0.x:
            x = x1
        else:
            print "p0: " + str(p0) + " | p1: " + str(p1) + " | x0: " + str(x0) + " | x1: " + str(x1)
            raise Exception("Point.intersect: invalid input")

        m = (p1.y - p0.y)/(p1.x - p0.x)
        y = m * (x - p0.x) + p0.y

        return Point([x, y], None)

    def dist2(self, other):
        return ((self.x - other.x)**2) + ((self.y - other.y)**2)


class Polyline(object):
    class_count = 0;
    def __init__(self, points=None, points_string=None):
        if points is None and points_string is None:
            self.points = []
            # call self.close_shape() after all points have been appended!
            self.top, self.right, self.bottom, self.left = [None for _ in range(4)]
        elif points:
            self.points = points
            # sets bounds
            self.top, self.right, self.bottom, self.left = self.set_bounds()
        else:
            pattern = re.compile("\d+\.\d+\,\d+\.\d+")
            pn = pattern.findall(points_string)
            self.points = [Point(map(float, p.split(",")), self) for p in pn]
            # sets bounds
            self.top, self.right, self.bottom, self.left = self.set_bounds()

        self.id = "polyline" + str(Polyline.class_count)
        Polyline.class_count += 1

    def close_shape(self):
        self.top, self.right, self.bottom, self.left = self.set_bounds()

    def closest(self, points):
        # returns points [p1, p2], where {p1:this} and {p2: points}
        min_dist2 = float("inf")
        min_points = []
        for p1 in self.points:
            for p2 in points:
                d = p1.dist2(p2)
                if d < min_dist2:
                    min_dist2 = d
                    min_points = [p1, p2]
        return min_points

    def set_bounds(self):
        x_sorted = sorted(self.points, key=lambda p: p.x)
        y_sorted = sorted(self.points, key=lambda p: p.y)

        # returns top, right, bottom, left boundary points
        return y_sorted[0], x_sorted[-1], y_sorted[-1], x_sorted[0]

    def append(self, point):
        self.points.append(point)

    def __repr__(self):
        return "Polyline(" + " ".join(map(str, self.points)) + ")"

    def to_xml(self, doc):
        node = doc.createElement("polyline")
        node.setAttribute("stroke","#ff0000")
        node.setAttribute("fill", "none")
        node.setAttribute("stroke-width", "0.1")
        node.setAttribute("points", " ".join(map(str, self.points)))
        return node

    def get_segments(self, x0, x1):
        # returns a list of polylines containing segments of this polygon between x0 and x1

        if x0 > self.right.x or x1 < self.left.x:
            return []

        elif x0 < self.left.x and x1 > self.right.x:
            # entire shape is enclosed within boundaries,
            return [self]


        polylines = []

        prev = self.points[0]
        prev_in = prev.btw(x0, x1)

        if prev_in:
            polylines.append(Polyline(points=[prev]))

        elif len(self.points) >= 2 and x0 > self.points[0].x and x1 < self.points[1].x:
            prev = Point.intersect(self.points[0], self.points[1], x0, x1)
            polylines.append(Polyline(points=[prev]))
            prev_in = True

        for i in range(1, len(self.points)):
            current = self.points[i]
            current_in = current.btw(x0, x1)

            if current_in and prev_in:
                polylines[-1].append(current)

            elif current_in and (not prev_in):
                polylines.append(Polyline(points=[Point.intersect(current, prev, x0, x1), current]))

            elif (not current_in) and prev_in:
                polylines[-1].append(Point.intersect(current, prev, x0, x1))
                polylines[-1].close_shape()

            prev = current
            prev_in = current_in

        return polylines


class Polygon(object):
    class_count = 0;
    # points is a string of points
    def __init__(self, points):
        pattern = re.compile("\d+\.\d+\,\d+\.\d+")
        pn = pattern.findall(points)
        self.points = [Point(map(float, p.split(",")), self) for p in pn]

        # bounds
        self.top, self.right, self.bottom, self.left = self.set_bounds()
        self.id = "polygon" + str(Polygon.class_count)
        Polygon.class_count += 1

    def set_bounds(self):
        x_sorted = sorted(self.points, key=lambda p: p.x)
        y_sorted = sorted(self.points, key=lambda p: p.y)

        # returns top, right, bottom, left boundary points
        return y_sorted[0], x_sorted[-1], y_sorted[-1], x_sorted[0]

    def to_polyline(self):
        # converting Polygon to Polyline :: append first point at the end.
        return Polyline(points=(self.points + [self.points[0]]))

    def closest(self, points, matched):
        # returns points [p1, p2], where {p1:this} and {p2: points}
        min_dist2 = float("inf")
        min_points = []
        for p1 in self.points:
            for p2 in points:
                if [self.id, p2.pl.id] not in matched:
                    d = p1.dist2(p2)
                    if d < min_dist2:
                        min_dist2 = d
                        min_points = [p1, p2]
                else:
                    print "MATCHEDDDDDDDDDD\n\n\n\n"
        return min_points

    def get_segments(self, x0, x1):
        # returns a list of polylines containing segments of this polygon between x0 and x1

        if x0 > self.right.x or x1 < self.left.x:
            return []

        elif x0 < self.left.x and x1 > self.right.x:
            # entire shape is enclosed within boundaries,
            return [self.to_polyline()]

        polylines = []

        prev = self.points[-1]
        prev_in = prev.btw(x0, x1)

        if prev_in:
            polylines.append(Polyline(points=[prev]))

        for current in self.points:
            current_in = current.btw(x0, x1)

            if current_in and prev_in:
                polylines[-1].append(current)

            elif current_in and (not prev_in):
                polylines.append(Polyline(points=[Point.intersect(current, prev, x0, x1), current]))

            elif (not current_in) and prev_in:
                polylines[-1].append(Point.intersect(current, prev, x0, x1))

            prev = current
            prev_in = current_in

        return polylines

    def to_xml(self, doc):
        node = doc.createElement("polygon")
        node.setAttribute("stroke","#ff0000")
        node.setAttribute("fill", "none")
        node.setAttribute("stroke-width", "0.1")
        node.setAttribute("points", " ".join(map(str, self.points)))
        return node

class Polygons(object):
    # wrapper for a list of polygons
    def __init__(self, filename):
        self.filename = filename
        self.doc = minidom.parse(filename + "_orig.svg")
        poly = self.doc.getElementsByTagName("polygon")
        polylines = self.doc.getElementsByTagName("polyline")

        self.polygons = [Polygon(p.getAttribute("points")) for p in poly if p.getAttribute("display") != "none"]
        self.polygons.extend([Polyline(points_string=p.getAttribute("points")) for p in polylines if p.getAttribute("display") != "none"])

        #sorted list of rightmost and leftmost points of all poligons
        self.left, self.right = self.set_bounds()
        self.sorted = sorted(self.polygons, key=lambda pl: pl.right.x, reverse=True)  # sorted right -> left

        self.polygons.extend(self.connect())


    def set_bounds(self):
        left = sorted([p.left for p in self.polygons], key=lambda pt: pt.x)[0]
        right = sorted([p.right for p in self.polygons], key=lambda pt: pt.x)[-1]
        return left, right

    def connect(self):
        # returns: list of polylines connecting polygons
        # points = [self.right]  # start @ right-most point
        polylines = []
        matched = []
        for i in range(1, len(self.sorted)):
            print i 
            #poligon starting from the right, find the closest point on the right
            pl = self.sorted[i]

            all_points = [pt for p in self.polygons if p != pl for pt in p.points]
            closest = pl.closest(all_points, matched)
            polylines.append(Polyline(points=closest))
            if (closest[1].pl != None):
                matched.extend([[pl.id, closest[1].pl.id], [closest[1].pl.id, pl.id]])
            print matched
            # points.extend(pl.points)
        return polylines

    def get_segments(self, x0, x1):
        segments = []
        for p in self.polygons:
            segments.extend(p.get_segments(x0, x1))
        return segments

    def get_intervals(self, x0, x):
        x0 = mm_to_pt(x0)
        x = mm_to_pt(x)
        s = x0
        intervals = [[0, x0]]
        while s < self.right.x:
            s1 = s + x
            intervals.append([s, s1])
            s = s1
        return intervals

    def segment(self, x0, x):
        intervals = self.get_intervals(x0, x)
        segments = []
        for i in intervals:
            segments.append(self.get_segments(i[0], i[1]))
        print "# segments: " + str(len(segments))
        return segments

    def write_segments(self, x0, x, output):
        segments = self.segment(x0, x)
        doc = minidom.Document()
        svg = doc.createElement("svg")
        svg.setAttribute("version", "1.1")
        svg.setAttribute("width", self.doc.getElementsByTagName("svg")[0].getAttribute("width"))
        svg.setAttribute("height", self.doc.getElementsByTagName("svg")[0].getAttribute("height"))
        doc.appendChild(svg)

        for segment in segments:
            g = doc.createElement("g")
            for pl in segment:
                g.appendChild(pl.to_xml(doc))
            svg.appendChild(g)

        doc.writexml(open(output, "w"))
        doc.unlink()

    def split(self):
        self.write_segments(174, 200, self.filename + "_split.svg")

    def write_connected(self):
        output = self.filename + "_connect.svg"
        doc = minidom.Document()
        svg = doc.createElement("svg")
        svg.setAttribute("version", "1.1")
        svg.setAttribute("width", self.doc.getElementsByTagName("svg")[0].getAttribute("width"))
        svg.setAttribute("height", self.doc.getElementsByTagName("svg")[0].getAttribute("height"))
        doc.appendChild(svg)

        for pl in self.polygons:
            svg.appendChild(pl.to_xml(doc))

        doc.writexml(open(output, "w"))
        doc.unlink()


def mm_to_pt(x):
    return x * 2.8347
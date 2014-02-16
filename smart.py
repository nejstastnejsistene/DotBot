from collections import defaultdict

class Point(object):
    def __init__(self, row, col):
        self.row = row
        self.col = col
    def to_tuple(self):
        return self.row, self.col
    def __repr__(self):
        return '({}, {})'.format(self.row, self.col)

class ConvexHull(object):
    def __init__(self, p1, p2):
        self.p1 = p1
        self.p2 = p2

def find_convex_hull(path):
    minr, minc = path[0]
    maxr, maxc = path[0]
    for r, c in path[1:]:
        if r < minr: minr = r
        if c < minc: minc = c
        if r > maxr: maxr = r
        if c > maxc: maxc = c
    return ConvexHull(Point(minr, minc), Point(maxr, maxc))

def find_encircled_dots(path):
    lookup = defaultdict(lambda: False)
    for row, col in path:
        lookup[row, col] = True

    hull = find_convex_hull(path)
    for row in range(hull.p1.row + 1, hull.p2.row):
        col = hull.p1.col
        while not lookup[row, col]: lookup[row, col] = True; col += 1
        col = hull.p2.col
        while not lookup[row, col]: lookup[row, col] = True; col -= 1
    for col in range(hull.p1.col + 1, hull.p2.col):
        row = hull.p1.row
        while not lookup[row, col]: lookup[row, col] = True; row += 1
        row = hull.p2.row
        while not lookup[row, col]: lookup[row, col] = True; row -= 1

    encircled = []
    for row in range(hull.p1.row + 1, hull.p2.row):
        for col in range(hull.p1.col + 1, hull.p2.col):
            if not lookup[row, col]:
                encircled.append(Point(row, col))

    return encircled

from PIL import Image

purple = 153, 93, 181
blue = 138, 188, 255
red = 235, 93, 70
yellow = 229, 219, 37
green = 138, 233, 145
white = 255, 255, 255

filename = 'screenshot.png'
img = Image.open(filename).convert('RGB')
px = img.load()
w, h = img.size

def get_grid_start():
    for x in range(w):
        for y in range(h):
            if px[x, y] == white:
                return x, y


def get_grid_dimensions():
    x0, y0 = get_grid_start()
    for x in range(x0, w):
        if px[x, y0] != white:
            x1 = x
            break
    for y in range(y0, h):
        if px[x0, y] != white:
            y1 = y
            break
    return x0, y0, x1, y1

def group_edges(func):
    def newfunc():
        edge = []
        for point in func():
            if point is None:
                if edge:
                    yield edge
                edge = []
            else:
                edge.append(point)
    return newfunc

@group_edges
def vertical_edges():
    x0, y0, x1, y1 = get_grid_dimensions()
    for y in range(y0, y1):
        for x in range(x0, x1):
            if px[x, y] != white:
                yield x, y
                break
        else:
            yield None

@group_edges
def horizontal_edges():
    x0, y0, x1, y1 = get_grid_dimensions()
    for x in range(x0, x1):
        for y in range(y0, y1):
            if px[x, y] != white:
                yield x, y
                break
        else:
            yield None

def scale_coord(x, y):
    return int(x * 1343. / w), int(y * 2239. / h)


def get_grid():
    for h in horizontal_edges():
        for v in vertical_edges():
            x, _ = h[len(h) / 2]
            _, y = v[len(v) / 2]
            yield px[x, y], scale_coord(x, y)

def hue(r, g, b):
    return math.atan2(math.sqrt(3)*(g-b), 2*r-g-b) % (2*math.pi)

def get_color(color):
    return int(5 * hue(*color) / (2 * math.pi) + 0.5)

grid = list(get_grid())
colors = list(set(color for color, _ in grid))
for color, _ in grid:
    import math
    print get_color(color)

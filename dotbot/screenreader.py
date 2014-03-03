import math
from functools import wraps
from PIL import Image

white = 255, 255, 255

nexus_7_width = 1343.0
nexus_7_height = 2239.0


class ScreenReadingException(Exception):
    pass


class GameScreen(object):
    def __init__(self, px, w, h):
        self.colors, self.coords = zip(*get_grid(px, w, h))


def readscreen(screenshot):
    img = Image.open(screenshot).convert('RGB')
    px = img.load()
    w, h = img.size
    return GameScreen(px, w, h)


def get_grid(px, w, h):
    for h_edges in horizontal_edges(px, w, h):
        for v_edges in vertical_edges(px, w, h):
            x, _ = h_edges[len(h_edges) / 2]
            _, y = v_edges[len(v_edges) / 2]
            yield get_color(px[x, y]), scale_coord(w, h, x, y)


def group_edges(func):
    @wraps(func)
    def newfunc(*args, **kwargs):
        edge = []
        for point in func(*args, **kwargs):
            if point is None:
                if edge:
                    yield edge
                edge = []
            else:
                edge.append(point)
    return newfunc


@group_edges
def vertical_edges(px, w, h):
    x0, y0, x1, y1 = get_grid_dimensions(px, w, h)
    for y in range(y0, y1):
        for x in range(x0, x1):
            if px[x, y] != white:
                yield x, y
                break
        else:
            yield None


@group_edges
def horizontal_edges(px, w, h):
    x0, y0, x1, y1 = get_grid_dimensions(px, w, h)
    for x in range(x0, x1):
        for y in range(y0, y1):
            if px[x, y] != white:
                yield x, y
                break
        else:
            yield None


def get_grid_dimensions(px, w, h):
    x0, y0 = get_grid_start(px, w, h)
    for x in range(x0, w):
        if px[x, y0] != white:
            x1 = x
            break
    else:
        raise ScreenReadingException
    for y in range(y0, h):
        if px[x0, y] != white:
            y1 = y
            break
    else:
        raise ScreenReadingException
    return x0, y0, x1, y1


def get_grid_start(px, w, h):
    for x in range(w):
        for y in range(h):
            if px[x, y] == white:
                return x, y
    raise ScreenReadingException


def get_color(rgb):
    '''Round a hue to its nearest color in the range [0,5).'''
    return int(5 * hue(*rgb) / (2 * math.pi) + 0.5)


def hue(r, g, b):
    '''Calculate the hue of a color, in the range [0,2pi).'''
    return math.atan2(math.sqrt(3)*(g-b), 2*r-g-b) % (2*math.pi)


def scale_coord(w, h, x, y):
    return int(x * nexus_7_width / w), int(y * nexus_7_height / h)


if __name__ == '__main__':
    filename = 'screenshot.png'
    img = Image.open(filename).convert('RGB')
    px = img.load()
    w, h = img.size

    grid = list(get_grid())
    colors = list(set(color for color, _ in grid))
    for color, _ in grid:
        print get_color(color)

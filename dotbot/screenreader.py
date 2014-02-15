from collections import namedtuple
from PIL import Image

__all__ == 'read_game_screen',


GameScreenInfo = namedtuple('GameScreenInfo', \
        'coords colors menu time_freeze shrinker expander')


black = 0, 0, 0
white = 255, 255, 255

def scan_row_horizontally(img, y):
    for x in range(img.size[0]):
        if img.getpixel((x, y)) not in (white, black):
            return x

def iter_dots(img):
    edge = []
    for y in range(img.size[1]):
        if img.getpixel((0, y)) in (white, black):
            x = scan_row_horizontally(img, y) 
            if x is not None:
                edge.append((x, y))
            elif edge:
                radius = len(edge) / 2
                x, y = edge[radius]
                yield x + radius, y
                edge = [] 

def find_menu(img):
    w, h = img.size
    for y in range(h):
        if img.getpixel((0, y)) in (white, black):
            break
    return w / 4, y / 2

def find_powerups(img):
    w, h = img.size
    for y in range(h / 2, h):
        if img.getpixel((0, y)) not in (white, black):
            break
    y = (h + y) / 2
    return (w / 4, y), (w / 2, y), (3 * w / 4, y)

def read_game_screen(filename):
    img = Image.open(filename)
    dots = iter_dots(img)
    x, y = dots.next()
    dist = dots.next()[1] - y
    coords = [[(x+dist*c,y+dist*r) for c in range(6)] for r in range(6)]
    colors = [[img.getpixel(coord) for coord in row] for row in coords]
    menu = find_menu(img)
    powerups = find_powerups(img)
    return GameScreenInfo(coords, colors, menu, *powerups)

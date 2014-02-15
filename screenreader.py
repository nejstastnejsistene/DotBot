from PIL import Image

black = 0, 0, 0
white = 255, 255, 255

def scan_row(img, y):
    for x in range(img.size[0]):
        if img.getpixel((x, y)) not in (white, black):
            return x

def iter_dots(img):
    edge = []
    for y in range(img.size[1]):
        if img.getpixel((0, y)) in (white, black):
            x = scan_row(img, y) 
            if x is not None:
                edge.append((x, y))
            elif edge:
                radius = len(edge) / 2
                x, y = min(edge, key=lambda x: x[0])
                yield x + radius, y
                edge = [] 

def get_dots(filename):
    img = Image.open(filename)
    dots = iter_dots(img)
    x, y = dots.next()
    dist = dots.next()[1] - y
    coords = [[(x+dist*c,y+dist*r) for c in range(6)] for r in range(6)]
    colors = [[img.getpixel(coord) for coord in row] for row in coords]
    return coords, colors

import itertools
coords, colors = get_dots('images/game-screen.png')
print set(itertools.chain(*colors))

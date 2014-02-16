import random
from StringIO import StringIO

import smart


class DotsGrid(object):

    clear_code = '\033[0m'
    color_codes = [ '\033[31m' # Red.
                  , '\033[32m' # Green.
                  , '\033[33m' # Yellow.
                  , '\033[35m' # Magenta.
                  , '\033[36m' # Cyan.
                  ]

    def __init__(self, colors=None):
        self.colors = colors or [random.randrange(5) for i in range(36)]
        self.score = 0

    def __getitem__(self, rc):
        row, col = rc
        return self.colors[6 * row + col]

    def __setitem__(self, rc, color):
        row, col = rc
        self.colors[6 * row + col] = color

    def apply_path(self, path):
        unique_coords = set(path)
        
        if len(path) != len(unique_coords):
            # It formed a square, clear all dots.
            encircled = smart.find_encircled_dots(path)
            for dot in smart.find_encircled_dots(path):
                row, col = dot.to_tuple()
                if self[row, col] != self[path[0]]:
                    self.shrink(row, col)
            self.expand(*path[0])
        else:
            # Otherwise just remove the dots in the path.
            # Sort them by row first.
            dots = sorted(unique_coords, key=lambda x: x[0])
            for row, col in dots:
                self.shrink(row, col)

    def shrink(self, row, col, exclude=None):
        self.score += 1
        for r in range(row, 0, -1):
            self[r, col] = self[r - 1, col] 
        choices = filter(lambda x: x != exclude, range(5))
        self[0, col] = random.choice(choices)

    def expand(self, row, col):
        color = self[row, col]
        for row in range(6):
            for col in range(6):
                if self[row, col] == color:
                    self.shrink(row, col, exclude=color)

    def __repr__(self):
        s = StringIO()
        for r in range(6):
            for c in range(6):
                s.write(self.color_codes[self.colors[6*r+c]])
                s.write('@ ')
            s.write('\n')
        s.write(self.clear_code)
        return s.getvalue()


if __name__ == '__main__':

    grid = [4, 2, 0, 2, 1, 0, \
            3, 2, 1, 4, 4, 4, \
            3, 3, 3, 4, 2, 0, \
            3, 0, 3, 2, 3, 3, \
            3, 3, 3, 4, 2, 1, \
            3, 1, 4, 2, 1, 4]

    loop = (2, 0), \
           (2, 1), \
           (2, 2), \
           (3, 2), \
           (4, 2), \
           (4, 1), \
           (4, 0), \
           (3, 0), \
           (2, 0)

    emu = DotsGrid(grid)
    print emu
    emu.apply_path(loop)
    print emu

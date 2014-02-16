import random
from StringIO import StringIO


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
    print DotsGrid()

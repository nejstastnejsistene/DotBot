c1r1 = 0xd7, 0x20d
c1r2 = 0xd7, 0x2a8
dist = c1r2[1] - c1r1[1]

def get_coord(row, col):
    return c1r1[0] + col * dist, c1r1[1] + row * dist

sleep_time = int(0.01 * 1e9)
reran_path = '/data/local/replay'
script_path = '/data/local/events.txt'

class RERANScript(object):

    def __init__(self, device=0):
        self.device = device
        self.events = []
        self.x = None
        self.y = None

    def event(self, type, code, value):
        fmt = '{}\n{},{},{},{}\n'
        output = fmt.format(sleep_time, self.device, type, code, value)
        self.events.append(output)

    def setpos(self, x, y, finger_down=False):
        if x != self.x:
            self.event(3, 0, x)
        if y != self.y:
            self.event(3, 1, y)
        if finger_down:
            self.finger_down()
        else:
            self.seperator()

    def finger_down(self):
        self.event(1, 330, 1)
        self.seperator()

    def finger_up(self):
        self.event(1, 330, 0)
        self.seperator()

    def seperator(self):
        self.event(0, 0, 0)

    def click(self, x, y):
        self.setpos(x, y, True)
        self.finger_up()

    def gesture(self, coords, n=10):
        last_x, last_y = coords[0]
        self.setpos(last_x, last_y, True)
        for x, y in coords[1:]:
            for i in range(n):
                self.setpos(last_x + i * (x - last_x) / n,
                            last_y + i * (y - last_y) / n)
            self.setpos(x, y)
            last_x, last_y = x, y
        #self.finger_up()

        for x, y in coords[::-1][:]:
            for i in range(n):
                self.setpos(last_x + i * (x - last_x) / n,
                            last_y + i * (y - last_y) / n)
            self.setpos(x, y)
            last_x, last_y = x, y

    def __str__(self):
        return '{}\n'.format(len(self.events)) + ''.join(self.events)

import sys

script = RERANScript()
script.gesture(
        [get_coord(*map(int, line.split(','))) for line in sys.stdin])

import tempfile
from commands import getoutput

with tempfile.NamedTemporaryFile() as tmp:
    print script
    tmp.write(str(script))
    tmp.flush()
    print getoutput('adb push {} {}'.format(tmp.name, script_path))
print getoutput('adb shell {} {}'.format(reran_path, script_path))

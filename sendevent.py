from evdev.ecodes import *

#240 613
#240 792

def get_coord(row, col):
    return 240 + 179 * col, 613 + 179 * row

sleep_time = int(0.0025 * 1e9)
reran_path = '/data/local/DotBot/replay'
script_path = '/data/local/DotBot/events.txt'

class RERANScript(object):

    def __init__(self, device=0):
        self.device = device
        self.events = []

    def event(self, type, code, value):
        fmt = '{}\n{},{},{},{}\n'
        output = fmt.format(sleep_time, self.device, type, code, value)
        self.events.append(output)

    def setpos(self, x, y, finger_down=False):
        self.event(EV_ABS, ABS_X, x)
        self.event(EV_ABS, ABS_Y, y)
        if finger_down:
            self.finger_down()
        else:
            self.seperator()

    def finger_down(self):
        self.event(EV_KEY, BTN_TOUCH, 1)
        self.seperator()

    def finger_up(self):
        self.event(EV_KEY, BTN_TOUCH, 0)
        self.seperator()

    def seperator(self):
        self.event(EV_SYN, SYN_REPORT, 0)

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
        self.finger_up()

    def __str__(self):
        return '{}\n'.format(len(self.events)) + ''.join(self.events)


class RERANScriptMT(RERANScript):

    def __init__(self):
        RERANScript.__init__(self)
        self.tracking_id = 0;

    def setpos(self, x, y, finger_down=False):
        if finger_down:
            self.finger_down()
        self.event(EV_ABS, ABS_MT_POSITION_X, x)
        self.event(EV_ABS, ABS_MT_POSITION_Y, y)
        self.seperator()

    def finger_down(self):
        self.event(EV_ABS, ABS_MT_TRACKING_ID, self.tracking_id)
        self.tracking_id += 1
        #self.seperator()

    def finger_up(self):
        self.event(EV_ABS, ABS_MT_TRACKING_ID, -1)
        self.seperator()

    def seperator(self):
        self.event(EV_SYN, SYN_REPORT, 0)

import sys

script = RERANScriptMT()
script.gesture(
        [get_coord(*map(int, line.split())) for line in sys.stdin])

import tempfile
import subprocess

def call(*args):
    assert subprocess.call(args) == 0

with tempfile.NamedTemporaryFile() as tmp:
    tmp.write(str(script))
    tmp.flush()
    call('adb', 'push', tmp.name, script_path)
    call('adb', 'shell', reran_path, script_path)

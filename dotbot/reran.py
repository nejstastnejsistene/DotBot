import contextlib
from evdev.ecodes import *


default_sleep_time = int(0.0025 * 1e9)
long_sleep_time = 100 * default_sleep_time


class RERANScript(object):

    def __init__(self, device=0):
        self.device = device
        self.events = []

    def event(self, type, code, value, sleep_time=default_sleep_time):
        fmt = '{}\n{},{},{},{}\n'
        output = fmt.format(sleep_time, self.device, type, code, value)
        self.events.append(output)

    def __str__(self):
        return '{}\n'.format(len(self.events)) + ''.join(self.events)

    @contextlib.contextmanager
    def user_action(self, x, y):
        self.start_action(x, y)
        yield
        self.end_action()

    @contextlib.contextmanager
    def packet(self):
        yield
        self.sync()

    def start_action(self, x, y):
        with self.packet():
            self.setpos(x, y)
            self.event(EV_KEY, BTN_TOUCH, 1)

    def end_action(self):
        with self.packet():
            self.event(EV_KEY, BTN_TOUCH, 0, long_sleep_time)

    def setpos(self, x, y):
        self.event(EV_ABS, ABS_X, x)
        self.event(EV_ABS, ABS_Y, y)

    def sync(self):
        self.event(EV_SYN, SYN_REPORT, 0)

    def click(self, x, y):
        with self.user_action(x, y): pass

    def gesture(self, coords):
        coords = iter(coords)
        with self.user_action(*next(coords)):
            for coord in coords:
                with self.packet():
                    self.setpos(*coord)


class RERANScriptMT(RERANScript):

    def __init__(self, *args, **kwargs):
        RERANScript.__init__(self, *args, **kwargs)
        self.tracking_id = 0

    def start_action(self, x, y):
        with self.packet():
            self.event(EV_ABS, ABS_MT_TRACKING_ID, self.tracking_id)
            self.setpos(x, y)
        self.tracking_id += 1

    def end_action(self):
        with self.packet():
            self.event(EV_ABS, ABS_MT_TRACKING_ID, -1, long_sleep_time)

    def setpos(self, x, y):
        self.event(EV_ABS, ABS_MT_POSITION_X, x)
        self.event(EV_ABS, ABS_MT_POSITION_Y, y)


def interpolate(coords, n=10):
    coords = iter(coords)
    last_x, last_y = next(coords)
    yield last_x, last_y
    for x, y in coords:
        for i in range(1, n + 1):
            yield (last_x + i * (x - last_x) / n,
                   last_y + i * (y - last_y) / n)
        last_x, last_y = x, y

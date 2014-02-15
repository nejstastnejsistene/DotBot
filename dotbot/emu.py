from ai import path_to_pixels
from commands import getoutput

device = '/dev/input/event0'

def event(type, code, value):
    cmd = 'sendevent {} {} {} {}'.format(device, type, code, value)
    print cmd
    getoutput('adb shell ' + cmd)

def set_pos(x=None, y=None):
    assert x or y
    if x is not None:
        event(3, 0, int(x))
    if y is not None:
        event(3, 1, int(y))
    seperator()

def finger_down():
    event(1, 330, 1)
    seperator()

def finger_up():
    event(1, 330, 0)
    seperator()

def seperator():
    event(0, 0, 0)

def click(coord):
    x, y = coord
    set_pos(x, y)
    finger_down()
    finger_up()

def swipe(path, n=4):
    set_pos(*path[0])
    finger_down()
    last_x, last_y = path[0]
    for x, y in path:
        dx = x - last_x
        dy = y - last_y
        for i in range(1, n+1):
            set_pos(last_x + dx * i / float(n), last_y + dy * i / float(n))
        last_x, last_y = x, y
    finger_up()

def screenshot(window, filename):
    getoutput('import -window {} {}'.format(window, filename))

def play_path(info, path):
    path = path_to_pixels(path, info.coords)
    if isinstance(path[0], basestring):
        if path[0] == 'time_freeze':
            click(info.time_freeze)
        elif path[0] == 'shrinker':
            click(info.shrinker)
        elif path[0] == 'expander':
            click(info.expander)
        click(path[1])
    else:
        swipe(path)

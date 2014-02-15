from commands import getoutput

device = '/dev/input/event0'

def event(type, code, value):
    cmd = 'sendevent {} {} {} {}'.format(device, type, code, value)
    print cmd
    getoutput('adb shell ' + cmd)

def set_pos(x=None, y=None):
    assert x or y
    if x is not None:
        event(3, 0, x)
    if y is not None:
        event(3, 1, y)
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

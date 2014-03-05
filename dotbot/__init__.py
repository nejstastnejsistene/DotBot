import contextlib
import tempfile
from StringIO import StringIO
from subprocess import Popen, PIPE, call

import paths
import reran
import screenreader


remote_dir = '/data/local/DotBot/'
screenshot_path = remote_dir + 'screenshot.raw'
reran_path      = remote_dir + 'replay'
script_path     = remote_dir + 'events.txt'


@contextlib.contextmanager
def screenshot():
    call(['adb', 'shell', 'screencap', screenshot_path])
    call(['adb', 'shell', 'gzip', screenshot_path])
    with tempfile.NamedTemporaryFile() as tmp:
        call(['adb', 'pull', screenshot_path + '.gz', tmp.name + '.gz'])
        call(['gunzip', '-f', tmp.name + '.gz'])
        yield tmp.name


def readscreen():
    with screenshot() as filename:
        return screenreader.readscreen(filename)


def DotBot(x, y):
    p = Popen(['bin/DotBot', '-'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
    out, err = p.communicate(x)
    retcode = p.wait()
    if retcode != 0:
        raise exception, err
    from screenreader import scale_coord
    w = 1200
    h = 1920
    coords = [scale_coord(w, h, *map(int, x.split())) for x in y.split('\n')]
    for r, c in paths.get_path(*map(eval, out.split())):
        yield coords[6 * c + r]


def sendevents(script):
    with tempfile.NamedTemporaryFile() as tmp:
        tmp.write(str(script))
        tmp.flush()
        call(['adb', 'push', tmp.name, script_path])
        call(['adb', 'shell', reran_path, script_path], stdout=open('/dev/null'))

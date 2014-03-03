import contextlib
import tempfile
from StringIO import StringIO
from subprocess import Popen, PIPE, call

import paths
import reran
import screenreader


remote_dir = '/data/local/DotBot/'
screenshot_path = remote_dir + 'screenshot.png'
reran_path      = remote_dir + 'replay'
script_path     = remote_dir + 'events.txt'


@contextlib.contextmanager
def screenshot():
    call(['adb', 'shell', 'screencap', '-p', screenshot_path])
    with tempfile.NamedTemporaryFile() as tmp:
        call(['adb', 'pull', screenshot_path, tmp.name])
        yield tmp.name


def readscreen():
    with screenshot() as filename:
        return screenreader.readscreen(filename)


def DotBot(screen):
    p = Popen(['bin/DotBot', '-'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
    out, err = p.communicate(' '.join(map(str, screen.colors)))
    retcode = p.wait()
    if retcode != 0:
        raise exception, err
    for r, c in paths.get_path(*map(eval, out.split())):
        yield screen.coords[6 * c + r]


def sendevents(script):
    with tempfile.NamedTemporaryFile() as tmp:
        tmp.write(str(script))
        tmp.flush()
        call(['adb', 'push', tmp.name, script_path])
        call(['adb', 'shell', reran_path, script_path])

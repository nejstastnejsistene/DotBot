#!/usr/bin/env python

import sys
import tempfile
from collections import defaultdict
from commands import getoutput
from subprocess import Popen, PIPE, call


def is_adjacent(a, b):
    ra, ca = a
    rb, cb = b
    return abs(rb-ra) + abs(cb-ca) == 1


def find_path(dots, start):
    return _find_path(dots - {start}, [start])


def _find_path(dots, path=None):
    if not dots:
        return path
    for dot in dots:
        if is_adjacent(dot, path[-1]):
            return _find_path(dots - {dot}, path + [dot])            


def get_path(has_cycle, bitmap):
    dots = set()
    for col in range(6):
        for row in range(6):
            if (bitmap >> (6 * col + row)) & 1:
                dots.add((row, col))
    num_neighbors = defaultdict(lambda: 0)
    for a in dots:
        for b in dots:
            if is_adjacent(a, b):
                num_neighbors[a] += 1
    if has_cycle:
        start = next(iter(dots))
    else:
        start = min(dots, key=num_neighbors.__getitem__)
    path = list(find_path(dots, start))
    for coord in path:
        yield coord
    if has_cycle:
        yield path[0]


def DotBot(colors, coords):
    p = Popen(['bin/DotBot', '-'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
    out, err = p.communicate(colors)
    retcode = p.wait()
    if retcode != 0:
        raise exception, err
    coords = [map(int, colors.split()) for colors in coords.split('\n')]
    for r, c in get_path(*map(eval, out.split())):
        yield coords[6 * c + r]


if __name__ == '__main__':
    output = getoutput('adb shell /data/local/DotBot/readscreen')
    try:
        colors, coords = output.split('\n', 1)
        with tempfile.NamedTemporaryFile() as f:
            for x, y in DotBot(colors, coords):
                f.write('{} {}\n'.format(x, y))
            f.flush()
            getoutput('adb push {} /data/local/DotBot/paths.txt'.format(f.name))
            getoutput('adb shell "/data/local/DotBot/sendevents < /data/local/DotBot/paths.txt"')
    except:
        if output:
            print >> sys.stderr, output
        sys.exit(1)

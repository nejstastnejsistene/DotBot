from collections import defaultdict


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
'''
Path format:
    (powerup_name, place_to_click)
    [<list of tuples to swipe between]
'''

from collections import defaultdict
from itertools import combinations


def path_to_pixels(path, coords):
    '''Change all coordinates from indices to pixels.'''
    if isinstance(path[0], basestring):
        r, c = path[1]
        return path[0], coords[r][c]
    else:
        return [coords[r][c] for r, c in path]

def super_simple(colors):
    '''Pop the top left corner with a shrinker.'''
    return 'shrinker', (0, 0)

def lj_algorithm(colors):
    '''Return the first path of length 2 we can find.'''
    for r in range(6):
        for c in range(5):
            if colors[r][c] == colors[r][c+1]:
                return (r, c), (r, c+1)
    for c in range(6):
        for r in range(5):
            if colors[r][c] == colors[r+1][c]:
                return (r, c), (r+1, c)


class Dot(object):
    def __init__(self, row, col, color):
        self.row = row
        self.col = col
        self.color = color
        self.neighbors = set()
    def __repr__(self):
        return '({}, {})'.format(self.row, self.col)

def partition(colors):
    dots_grid = [[Dot(r, c, colors[r][c]) for c in range(6)] for r in range(6)]
    partitions = []
    visited = defaultdict(lambda: False)
    for r in range(6):
        for c in range(6):
            if not visited[r, c]:
                partition = _partition(dots_grid, dots_grid[r][c], visited)
                connect_partition(partition)
                partitions.append(partition)
    return partitions

directions = (0, 1), (1, 0), (0, -1), (-1, 0)

def _partition(dots_grid, dot, visited):
    assert not visited[dot.row, dot.col]

    visited[dot.row, dot.col] = True
    acc = [dot]
    for dr, dc in directions:
        r, c = dot.row + dr, dot.col + dc
        if not (0 <= r < 6 and 0 <= c < 6):
            continue
        if visited[r, c]:
            continue
        if dots_grid[r][c].color != dot.color:
            continue
        acc += _partition(dots_grid, dots_grid[r][c], visited)
    return acc

def connect_partition(partition):
    for x, y in combinations(partition, 2):
        if is_adjacent(x, y):
            x.neighbors.add(y)
            y.neighbors.add(x)

def is_adjacent(x, y):
    '''Compute adjacency using the 1-norm.'''
    return abs(y.row-x.row) + abs(y.col-x.col) == 1

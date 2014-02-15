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
    def __iter__(self):
        yield self.row
        yield self.col
    def __repr__(self):
        return '({}, {})'.format(self.row, self.col)

class Edge(object):

    def __init__(self, p1, p2):
        self.p1 = p1
        self.p2 = p2

    def __eq__(self, other):
        a, b = self.p1.row, self.p1.col
        c, d = other.p2.row, other.p2.col
        return (a, b) == (c, d) or (b, a) == (c, d)

    def __hash__(self):
        r1, c1 = self.p1.row, self.p1.col
        r2, c2 = self.p2.row, self.p2.col
        return (1 << (r1*6+c1)) | (1 << (r2*6+c2))

    def __repr__(self):
        return 'Edge({}, {})'.format(self.p1, self.p2)


class PathBuildingException(Exception):
    pass


class Path(list):

    def __init__(self, head, *tail):
        list.__init__(self, (head,) + tail)
        self.edges = set()

    def append(self, item):
        edge = Edge(self[-1], item)
        if edge in self.edges:
            raise PathBuildingException
        self.edges.add(edge)
        list.append(self, item)

    def copy(self):
        cpy = Path(*self)
        cpy.edges = self.edges.copy()
        return cpy


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

def find_paths(colors):
    paths = []
    for p in partition(colors):
        for dot in p:
            paths += dfs(Path(dot))
    return paths

def dfs(path):
    paths = [path]
    for neighbor in path[-1].neighbors:
        try:
            new_path = path.copy()
            new_path.append(neighbor)
            paths += dfs(new_path)
        except PathBuildingException:
            continue
    return paths

def greedy_path(colors):
    return max(find_paths(colors), key=len)


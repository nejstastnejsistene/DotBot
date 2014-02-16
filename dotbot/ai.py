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
        return hash(self) == hash(other)

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

    def has_cycle(self):
        return len(set(self)) != len(self)

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
            if colors[r][c] is None:
                visited[r, c] = True
    for r in range(6):
        for c in range(6):
            if not visited[r, c]:
                partition = _partition(dots_grid, dots_grid[r][c], visited)
                connect_partition(partition)
                partitions.append(set(partition))
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

def find_paths(colors, use_dfs=True):
    paths = []
    for p in partition(colors):
        if len(p) > 4:
            cycle = find_cycle(p)
            if cycle is not None:
                paths.append(cycle)
                continue
        if use_dfs:
            for dot in p:
                paths += dfs(Path(dot))
    return paths

def find_cycle(partition):
    partition = partition.copy()
    neighbors = { node: node.neighbors.copy() for node in partition }

    # Remove "tails" from partition.
    endpoints = True
    while endpoints:
        endpoints = filter(lambda x: len(neighbors[x]) < 2, partition)
        for endpoint in endpoints:
            partition.remove(endpoint)
            for neighbor in endpoint.neighbors:
                neighbors[endpoint].discard(neighbor)
                neighbors[neighbor].discard(endpoint)

    # If it was just one long line it is now empty and has no cycle!
    if not partition:
        return

    # Weight the edges.
    weight_fn = lambda e: len(neighbors[e.p1]) + len(neighbors[e.p2])

    # Identify interior edges.
    interior_edges = set()
    for node in partition:
        weights = {}
        for x in neighbors[node]:
            edge = Edge(node, x)
            weights[edge] = weight_fn(edge)
        max_weight = max(weights.values())
        for e, weight in weights.items():
            if weight == max_weight \
                    and len(neighbors[e.p1]) > 2 \
                    and len(neighbors[e.p2]) > 2:
                interior_edges.add(e)

     # Remove interior edges.
    for seg in interior_edges:
        neighbors[e.p1].discard(e.p2)
        neighbors[e.p2].discard(e.p1)
 
    # Discard nodes without edges.
    partition = set(filter(neighbors.__getitem__, partition))

    # Find the largest path.
    path = Path(next(iter(partition)))
    return find_eularian_path(partition, neighbors, path)


def find_eularian_path(partition, neighbors, path):
    if path[0] == path[-1] and len(path) > 1:
        return path
    for node in neighbors[path[-1]]:
        try:
            new_path = path.copy()
            new_path.append(node)
            foo = find_eularian_path(partition, neighbors, new_path)
            if foo is not None:
                return foo
        except PathBuildingException:
            pass

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

def smart_path(colors, score=0, depth=2, weightBase=2):
    if depth < 0:
        return score, None

    score *= weightBase

    max_score = 0
    max_path = None

    paths = find_paths(colors, use_dfs=False)
    if not paths:
        paths = find_paths(colors)

    for path in paths:
        new_colors, new_score = apply_path(colors, path, score)
        weight, _ = smart_path(new_colors, new_score, depth - 1)
        if weight > max_score:
            max_score = weight
            max_path = path

    if len(max_path) == 1:
        return max_score, ('shrinker', max_path[0])
    else:
        return max_score, max_path

def apply_path(colors, path, score=0):
    colors = map(list, colors) # Deep copy.
    if path.has_cycle():
        color = path[0].color
        for r in range(6):
            for c in range(6):
                if colors[r][c] == color:
                    shrink(colors, r, c)
                    score += 1
    else:
        for node in sorted(path, key=lambda x: x.row):
            shrink(colors, node.row, node.col)
            score += 1
    return colors, score

def shrink(colors, row, col):
    for r in range(row, -1, -1):
        colors[r][col] = colors[r-1][col]
        colors[0][col] = None

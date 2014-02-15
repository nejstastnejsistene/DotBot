'''
Path format:
    (powerup_name, place_to_click)
    [<list of tuples to swipe between]
'''

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


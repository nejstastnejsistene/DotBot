import random
totes_score = 0

'''
Path format:
    (powerup_name, place_to_click)
    [<list of tuples to swipe between]
'''


def play_path(colors, path):
    if isinstance(path[0], basestring):
        r,c = path[1]
        colors = shrink( colors , r,c)
    else:
        colors = apply_path(colors, path)[0]
    return colors
    





def apply_path(old_colors, path, score=0):
    colors = map(list, old_colors) # Deep copy.
    if path.has_cycle():
        color = path[0].color
        choices = range(5)
        choices.remove(color)
        for r in range(6):
            for c in range(6):
                if colors[r][c] == color:
                    colors = shrink(colors, r, c, choices)
    else:
        for node in sorted(path, key=lambda x: x.row):
            colors = shrink(colors, node.row, node.col)
    return colors, score

def shrink(colors, row, col, choices = range(5)):
    global totes_score
    for r in range(row, -1, -1):
        colors[r][col] = colors[r-1][col]
        colors[0][col] = random.choice(choices)
    totes_score += 1
    return colors




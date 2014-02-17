from cpython cimport array

import random
from StringIO import StringIO


cdef struct ConvexHull:
    int r0, c0, r1, c1


cdef inline int getrow(int i):
    return i / 6

cdef inline int getcol(int i):
    return i % 6

cdef inline int getpoint(int r, int c):
    return 6 * r + c


cpdef int[:] path_from_py(path):
    return array.array('i', [getpoint(r, c) for r, c in path])

cpdef list path_to_py(int[:] path):
    return [(getrow(point), getcol(point)) for point in path]


cpdef int[:] random_board():
    return array.array('i', [random.randrange(5) for i in range(36)])


cpdef shrink(int[:] board, int point, int rand=True, int exclude=-1):
    cdef int row, col, r, x
    row, col = getrow(point), getcol(point)
    for r in range(row, 0, -1):
        board[getpoint(r, col)] = board[getpoint(r - 1, col)]
    if rand:
        choices = [x for x in range(5) if x != exclude]
        board[col] = random.choice(choices)
    else:
        board[col] = -1


cpdef expand(int[:] board, int point, int rand=True):
    cdef int color, i
    color = board[point] 
    for i in range(36):
        if board[i] == color:
            shrink(board, i, rand, exclude=color)


cdef ConvexHull find_convex_hull(int[:] path):
    cdef ConvexHull hull = ConvexHull(5, 5, 0, 0)
    cdef int point, row, col
    for point in path:
        row, col = getrow(point), getcol(point)
        if row < hull.r0: hull.r0 = row
        if col < hull.c0: hull.c0 = col
        if row > hull.r1: hull.r1 = row
        if col > hull.c1: hull.c1 = col
    return hull


cpdef int[:] get_encircled_dots(int[:] path):
    cdef ConvexHull hull = find_convex_hull(path)
    cdef int[:] is_outside = array.array('i', [False]*36)

    # Mark the path as on the outside.
    cdef int point
    for point in path:
        is_outside[point] = True

    # Try to "break inside" of the path from all 4 directions.
    cdef int row, col
    for row in range(hull.r0 + 1, hull.r1):
        col = hull.c0
        while not is_outside[getpoint(row, col)]:
            is_outside[getpoint(row, col)] = True; col += 1
        col = hull.c1
        while not is_outside[getpoint(row, col)]:
            is_outside[getpoint(row, col)] = True; col -= 1
    for col in range(hull.c0 + 1, hull.c1):
        row = hull.r0
        while not is_outside[getpoint(row, col)]:
            is_outside[getpoint(row, col)] = True; row += 1
        row = hull.r1
        while not is_outside[getpoint(row, col)]:
            is_outside[getpoint(row, col)] = True; row -= 1

    # Put the encircled dots into an array.
    cdef int[:] encircled = array.array('i', [0]*36)
    cdef int count = 0
    for row in range(hull.r0 + 1, hull.r1):
        for col in range(hull.c0 + 1, hull.c1):
            point = getpoint(row, col)
            if not is_outside[point]:
                encircled[count] = point
                count += 1
    return encircled[:count]


_color_codes = 31, 32, 33, 35, 36
def draw_board(board, dot=u'\u25cf'):
    s = StringIO()
    for r in range(6):
        for c in range(6):
            color = board[getpoint(r, c)]
            if color == -1:
                s.write('  ')
            else:
                s.write(' \033[')
                s.write(str(_color_codes[color]))
                s.write('m')
                s.write(dot)
                s.write('\033[0m')
        s.write('\n')
    return s.getvalue()

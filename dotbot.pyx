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

cpdef int[:] path_mask(int[:] path):
    cdef int[:] matrix = array.array('i', [False]*36)
    cdef int point
    for point in path:
        matrix[point] = True
    return matrix


cpdef int[:] random_board():
    return array.array('i', [random.randrange(5) for i in range(36)])


cpdef shrink(int[:] board, int point, int rand=True, int exclude=-1):
    cdef int row, col
    row, col = getrow(point), getcol(point)
    return _shrink(board[col::6], row, rand, exclude)

cdef _shrink(int[:] column, int row, int rand=True, int exclude=-1):
    cdef int r, x
    for r in range(row, 0, -1):
        column[r] = column[r - 1]
    if rand:
        choices = [x for x in range(5) if x != exclude]
        column[0] = random.choice(choices)
    else:
        column[0] = -1


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

cpdef int has_cycle(int[:] path):
    cdef int[:] seen = array.array('i', [False]*36)
    cdef int point
    for point in path:
        if seen[point]:
            return True
        seen[point] = True
    return False

cpdef int[:] get_encircled_dots(int[:] path):
    cdef ConvexHull hull = find_convex_hull(path)
    cdef int[:] is_outside = path_mask(path)

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
    cdef int point, count = 0
    for row in range(hull.r0 + 1, hull.r1):
        for col in range(hull.c0 + 1, hull.c1):
            point = getpoint(row, col)
            if not is_outside[point]:
                encircled[count] = point
                count += 1
    return encircled[:count]

cpdef perm_matrix(int[:] board):
    cdef int[:] matrix = array.array('i', [-1]*(6*64*6))
    cdef int col, perm, index, row
    cdef int[:] column
    for col in range(6):
        for perm in range(64):
            index = (64 * col + perm) * 6
            column = board[col::6].copy()
            for row in range(6):
                if perm & 1:
                    _shrink(column, row, rand=False)
                perm >>= 1
            matrix[index:index+6] = column
    return matrix

cpdef inline int[:] get_perm(int[:] matrix, int col, int perm):
    cdef index = (64 * col + perm) * 6
    return matrix[index:index+6]

cpdef apply_regular_path(int[:] matrix, int[:] path):
    cdef int[:] mask = path_mask(path)
    cdef int[:] perms = array.array('i', [0]*6)
    cdef int col, row
    for col in range(6):
        for row in range(6):
            if mask[getpoint(row, col)]:
                perms[col] |= (1 << row)
    return apply_permutation(matrix, perms)

cpdef int[:] apply_permutation(int[:] matrix, int[:] perms):
    cdef int[:] board = array.array('i', [0]*36)
    cdef int col
    for col in range(6):
        board[col::6] = get_perm(matrix, col, perms[col])
    return board


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

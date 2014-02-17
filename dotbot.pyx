from cpython.array cimport array

import random
from StringIO import StringIO


ctypedef signed char int_t
type_code = 'b'

cdef struct ConvexHull:
    int_t r0, c0, r1, c1


cdef inline int_t getrow(int_t point):
    return point / 6

cdef inline int_t getcol(int_t point):
    return point % 6

cdef inline int_t getpoint(int_t row, int_t col):
    return 6 * row + col


cpdef int_t[:] path_from_py(path):
    return array(type_code, [getpoint(r, c) for r, c in path])

cpdef list path_to_py(int_t[:] path):
    return [(getrow(point), getcol(point)) for point in path]

cpdef int_t[:] path_mask(int_t[:] path):
    '''Create a mask representing a path.'''
    cdef int_t[:] mask = array(type_code, [False]*36)
    cdef int_t point
    for point in path:
        mask[point] = True
    return mask


cpdef int_t[:] random_board():
    return array(type_code, [random.randrange(5) for i in range(36)])


cpdef translation_table(int_t[:] board):
    '''Construct a permutation translation matrix for a given board.

       This is structured as a 6x64x6 3D array. The first index is a
       column in range(6). The second index is a permutation in
       range(64), where the 6-bit bitmask of the permutation represents
       which dots in that column  have been removed. The array at
       matrix[i][j] is the result of the i'th column after shrinking the
       dots specified by the j'th permutation.
    '''
    cdef int_t[:] matrix = array(type_code, [-1]*(6*64*6))
    cdef int_t col, perm, row
    cdef int_t[:] column
    cdef int index
    for col in range(6):
        for perm in range(64):
            index = (64 * col + perm) * 6
            column = board[col::6].copy()
            for row in range(6):
                if perm & 1:
                    shrink_column(column, row, rand=False)
                perm >>= 1
            matrix[index:index+6] = column
    return matrix


cpdef int_t[:] apply_permutation(int_t[:] table, int_t[:] perms):
    '''Compute a permutation of a board, given its translation table.

       Given a board's permutation translation table `table` and a
       permutation matrix `perms`, compute the resulting board.

       Pseudocode example:

            table = translation_table(board)
            assert board == apply_permutation(table, [0]*6)
    '''
    cdef int_t[:] board = array(type_code, [0]*36)
    cdef int_t col
    cdef int index
    for col in range(6):
        index = (64 * col + perms[col]) * 6
        board[col::6] = table[index:index+6]
    return board


cpdef shrink(int_t[:] board, int_t point,
             int_t rand=True, int_t exclude=-1):
    '''Shrink a dot in a board, in-place.'''
    cdef int_t row, col
    row, col = getrow(point), getcol(point)
    return shrink_column(board[col::6], row, rand, exclude)


cdef shrink_column(int_t[:] column, int_t row,
                   int_t rand=True, int_t exclude=-1):
    '''Shrink a dot in a column, in-place.'''
    cdef int_t r, x
    for r in range(row, 0, -1):
        column[r] = column[r - 1]
    if rand:
        choices = [x for x in range(5) if x != exclude]
        column[0] = random.choice(choices)
    else:
        column[0] = -1


cpdef apply_path(int_t[:] board, int_t[:] table, int_t[:] path):
    '''Apply a path to board, given its translation table.'''
    cdef int_t[:] perms = array(type_code, [0]*6)
    cdef int_t [:] mask
    cdef int_t col, row, color

    # If there is a cycle, construct a permutation matrix for all dots
    # of the path's color, as well as any encircled dots.
    if has_cycle(path):
        color = board[path[0]]
        for col in range(6):
            for row in range(6):
                if board[getpoint(row, col)] == color:
                    perms[col] |= (1 << row)
        for point in get_encircled_dots(path):
            row, col = getrow(point), getcol(point)
            perms[col] |= (1 << row)

    # Otherwise, construct the matrix for just the path.
    else:
        mask = path_mask(path)
        for col in range(6):
            for row in range(6):
                if mask[getpoint(row, col)]:
                    perms[col] |= (1 << row)

    return apply_permutation(table, perms)


cpdef int_t has_cycle(int_t[:] path):
    '''Check if a path has a cycle by looking for repeated dots.'''
    cdef int_t[:] seen = array(type_code, [False]*36)
    cdef int_t point
    for point in path:
        if seen[point]:
            return True
        seen[point] = True
    return False


cdef ConvexHull get_convex_hull(int_t[:] path):
    '''Calculate the convex hull of a cyclic path.

       In other words, find the top left and bottom right points
       that form the smallest rectangle that contains the path.
       It does this by attempting to "enter" the cycle from all
       directions and marking the dots it passes as outside. The
       unmarked dots are on the inside of the path.
    '''
    cdef ConvexHull hull = ConvexHull(5, 5, 0, 0)
    cdef int_t point, row, col
    for point in path:
        row, col = getrow(point), getcol(point)
        if row < hull.r0: hull.r0 = row
        if col < hull.c0: hull.c0 = col
        if row > hull.r1: hull.r1 = row
        if col > hull.c1: hull.c1 = col
    return hull


cpdef int_t[:] get_encircled_dots(int_t[:] path):
    cdef ConvexHull hull = get_convex_hull(path)
    cdef int_t[:] is_outside = path_mask(path)

    # Try to "break inside" of the path from all 4 directions.
    cdef int_t row, col
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
    cdef int_t[:] encircled = array(type_code, [0]*36)
    cdef int_t point, count = 0
    for row in range(hull.r0 + 1, hull.r1):
        for col in range(hull.c0 + 1, hull.c1):
            point = getpoint(row, col)
            if not is_outside[point]:
                encircled[count] = point
                count += 1
    return encircled[:count]


_color_codes = 31, 32, 33, 35, 36
def draw_board(board, dot=u'\u25cf'):
    '''Return a colorful unicode string illustrating a board.'''
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

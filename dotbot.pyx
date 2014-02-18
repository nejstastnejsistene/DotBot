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
    '''Convert a list of coordinate pairs to an array of points.'''
    return array(type_code, [getpoint(r, c) for r, c in path])

cpdef list path_to_py(int_t[:] path):
    '''Convert an array of points to a list of coordinate pairs.'''
    return [(getrow(point), getcol(point)) for point in path]

cpdef int_t[:] path_mask(int_t[:] path, int_t bg=0, int_t fg=1):
    '''Create a mask representing a path.'''
    cdef int_t[:] mask = array(type_code, [bg]*36)
    cdef int_t point
    for point in path:
        mask[point] = fg
    return mask


cpdef int_t[:] random_board():
    '''Create a random board.'''
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
    cdef int_t[:] table = array(type_code, [-1]*(6*64*6))
    cdef int_t col, perm
    for col in range(6):
        for perm in range(64):
            compute_permutation(board, table, col, perm)
    return table


cdef inline compute_permutation(int_t[:] board, int_t[:] table,
                                int_t col, int perm):
    '''Compute the permutation of a column and put it in the table.'''
    cdef int index = (64 * col + perm) * 6
    cdef int_t[:] column = board[col::6].copy()
    cdef int_t row
    for row in range(6):
        if perm & 1:
            shrink_column(column, row)
        perm >>= 1
    table[index:index+6] = column


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


cpdef inline shrink(int_t[:] board, int_t point):
    '''Shrink a dot in a board, in-place.'''
    cdef int_t row, col
    row, col = getrow(point), getcol(point)
    return shrink_column(board[col::6], row)


cdef inline shrink_column(int_t[:] column, int_t row):
    '''Shrink a dot in a column, in-place.'''
    cdef int_t r, x
    for r in range(row, 0, -1):
        column[r] = column[r - 1]
    column[0] = -1


cpdef int_t[:] apply_path(int_t[:] board, int_t[:] table,
                          int_t[:] path, int fill=False):
    '''Apply a path to board using its permutation translation table.

       Optionally, you can specify to fill the empty dots.
    '''
    cdef int_t[:] perms = array(type_code, [0]*6)
    cdef int_t [:] mask
    cdef int_t col, row, color= -1

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

    cdef int_t[:] result = apply_permutation(table, perms)
    if fill:
        fill_empty_dots(result, exclude=color)
    return result


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
    '''Calculate the dots that are encircled by a cyclic path.

       This works by trying to "break into" the cycle from all directions
       while marking dots as outside. All unmarked dots are considered
       encircled by the path.
    '''
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


cdef fill_empty_dots(int_t[:] board, int exclude):
    '''Fill in empty spaces randomly, optionally excluding a color.'''
    cdef int_t point
    for point in range(36):
        if board[point] == -1:
            if exclude == -1:
                board[point] = random.randrange(5)
            else:
                choices = range(5)
                choices.remove(exclude)
                board[point] = random.choice(choices)


cpdef list get_partitions(int_t[:] board):
    '''Split a board into disjoint partitions of the same color.'''
    cdef int_t[:] visited = array(type_code, [False]*36)
    cdef int_t[:] partition
    cdef list partitions = []
    cdef int point
    for point in range(36):
        if not visited[point]:
            partition = _flood_fill(board, visited, point)
            if len(partition) > 0:
                partitions.append(partition)
    return partitions


cpdef int_t[:] _flood_fill(int_t[:] board, int_t[:] visited, int_t point):
    '''Find a parition using a flood fill algorithm.'''
    cdef int_t[:] partition = array(type_code, [0]*36)
    cdef int_t row, col, length = 0, color = board[point]
    cdef list stack = [point]

    while stack:
        visited[point] = True
        partition[length] = point
        length += 1

        row, col = getrow(point), getcol(point)

        point = getpoint(row - 1, col)
        if row > 0 and not visited[point] and board[point] == color:
            stack.append(point)

        point = getpoint(row + 1, col)
        if row < 5 and not visited[point] and board[point] == color:
            stack.append(point)

        point = getpoint(row, col - 1)
        if col > 0 and not visited[point] and board[point] == color:
            stack.append(point)

        point = getpoint(row, col + 1)
        if col < 5 and not visited[point] and board[point] == color:
            stack.append(point)

        point = stack.pop()

    return partition[:length]


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

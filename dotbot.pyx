from cpython cimport array

import random


cdef struct Point:
    int row, col


cpdef int[:] random_board():
    return array.array('i', [random.randrange(5) for i in range(36)])


def shrink(board, row, col, rand=True, exclude=-1):
    _shrink(board, Point(row, col), int(rand), exclude)

cdef _shrink(int[:] board, Point point, int rand=True, int exclude=-1):
    cdef int r, x
    for r in range(point.row, 0, -1):
        board[6*r+point.col] = board[6*(r-1)+point.col]
    if rand:
        choices = [x for x in range(5) if x != exclude]
        board[point.col] = random.choice(choices)
    else:
        board[point.col] = -1


def expand(board, row, col, rand=True):
    _expand(board, Point(row, col), rand)

cdef _expand(int[:] board, Point point, int rand=True):
    cdef int color, r, c
    color = board[6*point.row+point.col] 
    for r in range(6):
        for c in range(6):
            if board[6*r+c] == color:
                _shrink(board, Point(r, c), rand, exclude=color)


_color_codes = 31, 32, 33, 35, 36, 30
_board_fmt = (u' \033[{}m{dot}\033[0m'*6 + '\n')*6
def draw_board(board, dot=u'\u25cf'):
    codes = map(_color_codes.__getitem__, board)
    return _board_fmt.format(*codes, dot=dot)

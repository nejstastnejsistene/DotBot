import array
import dotbot

board = [1, 1, 1, 1, 1, 2, \
         1, 4, 1, 4, 1, 1, \
         1, 1, 1, 4, 1, 2, \
         3, 3, 1, 2, 1, 0, \
         4, 4, 1, 1, 1, 0, \
         4, 1, 3, 4, 1, 0]
board = array.array('i', board)

path = (0, 0), \
       (0, 1), \
       (0, 2), \
       (0, 3), \
       (0, 4), \
       (1, 4), \
       (2, 4), \
       (3, 4), \
       (4, 4), \
       (4, 3), \
       (4, 2), \
       (3, 2), \
       (2, 2), \
       (2, 1), \
       (2, 0), \
       (1, 0), \
       (0, 0), 
path = dotbot.path_from_py(path)
dots = dotbot.get_encircled_dots(path)
print dotbot.path_to_py(dots)
print dotbot.has_cycle(path)

print dotbot.draw_board(board)
m = dotbot.permutation_matrix(board)

board2 = dotbot.apply_path(board, m, path)
print dotbot.draw_board(board2)

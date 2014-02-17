import array
import dotbot

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

board = dotbot.random_board()
print dotbot.draw_board(board)
m = dotbot.perm_matrix(board)

print dotbot.draw_board(dotbot.path_mask(path))

board2 = dotbot.apply_regular_path(m, path)
print dotbot.draw_board(board2)

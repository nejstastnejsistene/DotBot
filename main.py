import array
import dotbot

board = [1, 1, 1, 1, 1, 2, \
         1, 4, 1, 4, 1, 1, \
         1, 1, 1, 4, 1, 2, \
         3, 3, 1, 2, 1, 0, \
         4, 4, 1, 1, 1, 0, \
         4, 1, 3, 4, 1, 0]
board = array.array('b', board)

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

print dotbot.draw_board(board)

'''
m = dotbot.translation_table(board)
board2 = dotbot.apply_path(board, m, path)

print dotbot.draw_board(board2)

path2 = (3, 5), (4, 5), (5, 5)
path2 = dotbot.path_from_py(path2)

m2 = dotbot.translation_table(board2)
board3 = dotbot.apply_path(board2, m2, path2)

print dotbot.draw_board(board3)
'''

for part in dotbot.get_partitions(board):
    print dotbot.draw_board(dotbot.path_mask(part, -1, board[part[0]]))

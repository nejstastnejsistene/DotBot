import dotbot

board = dotbot.random_board()
for i in range(5):
    for j in range(36):
        if board[j] == i:
            print dotbot.draw_board(board)
            dotbot.expand(board, j, False)
            break

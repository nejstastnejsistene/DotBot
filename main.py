import dotbot

while True:
    board = dotbot.random_board()
    print dotbot.draw_board(board)
    dotbot.expand(board, 0, 0, False)
    print dotbot.draw_board(board)
    import time;time.sleep(1)
